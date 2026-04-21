#include <lithium_http_server.hh>
#include <lithium_http_client.hh>
#include "symbols.hh"

using namespace li;

int main() {

  http_api my_api;

  my_api.get("/hello_world") = [&](http_request& request, http_response& response) {
    response.write("hello world.");
  };

  int port = 12334;

  http_serve(my_api, port, s::non_blocking, s::nthreads = 1);

  std::string persistent_req =
      "GET /hello_world HTTP/1.1\r\n"
      "Host: 127.0.0.1\r\n"
      "Connection: keep-alive\r\n"
      "\r\n";

  std::string short_lived_req =
      "GET /hello_world HTTP/1.1\r\n"
      "Host: 127.0.0.1\r\n"
      "Connection: close\r\n"
      "\r\n";

  auto sockets = http_benchmark_connect(50, port);
  float keep_alive_req_per_s = http_benchmark(sockets, 1, 2000, persistent_req);
  float short_lived_req_per_s = http_benchmark_new_connections(4, 2000, port, short_lived_req);

  std::cout << "persistent-connections: " << keep_alive_req_per_s << " req/s." << std::endl;
  std::cout << "new-connection-per-request: " << short_lived_req_per_s << " req/s." << std::endl;
  
  http_benchmark_close(sockets);
}
