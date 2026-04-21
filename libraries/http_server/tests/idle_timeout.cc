#include <cassert>
#include <chrono>
#include <cstring>
#include <string>
#include <thread>

#include <lithium_http_server.hh>
#include "symbols.hh"

#if defined(_WIN32)
#include <WS2tcpip.h>
#include <WinSock2.h>
using socket_type = SOCKET;
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
using socket_type = int;
#endif

using namespace li;

namespace {

void close_socket(socket_type fd) {
#if defined(_WIN32)
  closesocket(fd);
#else
  close(fd);
#endif
}

bool send_all(socket_type fd, const char* data, size_t len) {
  size_t sent = 0;
  while (sent < len) {
    int n = send(fd, data + sent, int(len - sent), 0);
    if (n <= 0)
      return false;
    sent += size_t(n);
  }
  return true;
}

std::string recv_once(socket_type fd, int timeout_ms) {
#if defined(_WIN32)
  DWORD tv = DWORD(timeout_ms);
  setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
#else
  timeval tv;
  tv.tv_sec = timeout_ms / 1000;
  tv.tv_usec = (timeout_ms % 1000) * 1000;
  setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif

  char buf[2048];
  int n = recv(fd, buf, sizeof(buf), 0);
  if (n <= 0)
    return std::string();
  return std::string(buf, size_t(n));
}

socket_type connect_localhost(int port) {
  socket_type fd = socket(AF_INET, SOCK_STREAM, 0);
#if defined(_WIN32)
  assert(fd != INVALID_SOCKET);
#else
  assert(fd >= 0);
#endif

  sockaddr_in addr;
  std::memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  int ret = connect(fd, (sockaddr*)&addr, sizeof(addr));
#if defined(_WIN32)
  assert(ret != SOCKET_ERROR);
#else
  assert(ret == 0);
#endif
  return fd;
}

} // namespace

int main() {
  constexpr int port = 12349;
  constexpr int idle_timeout_seconds = 1;

  http_api my_api;
  my_api.get("/hello_world") = [&](http_request&, http_response& response) {
    response.write("hello world.");
  };

  http_serve(my_api, port, s::non_blocking, s::idle_timeout_seconds = idle_timeout_seconds);

  {
    socket_type fd = connect_localhost(port);

    const std::string partial = "GET /hello_world HTTP/1.1\r\nHost: localhost\r\n";
    assert(send_all(fd, partial.data(), partial.size()));

    // Wait long enough for idle timeout + sweep interval to trigger.
    std::this_thread::sleep_for(std::chrono::milliseconds(2300));

    bool connection_closed = false;
    const std::string end_of_headers = "\r\n";
    if (!send_all(fd, end_of_headers.data(), end_of_headers.size()))
      connection_closed = true;

    if (!connection_closed) {
      std::string payload = recv_once(fd, 500);
      connection_closed = payload.empty();
    }

    assert(connection_closed);
    close_socket(fd);
  }

  {
    socket_type fd = connect_localhost(port);

    const std::string line1 = "GET /hello_world HTTP/1.1\r\n";
    const std::string line2 = "Host: localhost\r\n";
    const std::string line3 = "User-Agent: idle-timeout-test\r\n";
    const std::string line4 = "\r\n";

    assert(send_all(fd, line1.data(), line1.size()));
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    assert(send_all(fd, line2.data(), line2.size()));
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    assert(send_all(fd, line3.data(), line3.size()));
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    assert(send_all(fd, line4.data(), line4.size()));

    std::string response = recv_once(fd, 2000);
    assert(response.find("hello world.") != std::string::npos);

    close_socket(fd);
  }
}
