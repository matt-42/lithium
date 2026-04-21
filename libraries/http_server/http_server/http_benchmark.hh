#pragma once

#include <atomic>
#include <chrono>
#include <boost/context/continuation.hpp>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <li/http_server/timer.hh>
namespace ctx = boost::context;

namespace li {

namespace http_benchmark_impl {

inline void error(std::string msg) {
  perror(msg.c_str());
  exit(0);
}

} // namespace http_benchmark_impl

inline std::vector<int> http_benchmark_connect(int NCONNECTIONS, int port) {
  std::vector<int> sockets(NCONNECTIONS, 0);
  struct addrinfo hints, *serveraddr;

  /* socket: create the socket */
  for (int i = 0; i < sockets.size(); i++) {
    sockets[i] = socket(AF_INET, SOCK_STREAM, 0);
    if (sockets[i] < 0)
      http_benchmark_impl::error("ERROR opening socket");
  }

  /* build the server's Internet address */
	struct sockaddr_in server;
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_family = AF_INET;
	server.sin_port = htons( port );

  // Async connect.
  int epoll_fd = epoll_create1(0);

  auto epoll_ctl = [epoll_fd](int fd, int op, uint32_t flags) {
    epoll_event event;
    event.data.fd = fd;
    event.events = flags;
    ::epoll_ctl(epoll_fd, op, fd, &event);
    return true;
  };

  // Set sockets non blocking.
  for (int i = 0; i < sockets.size(); i++)
    fcntl(sockets[i], F_SETFL, fcntl(sockets[i], F_GETFL, 0) | O_NONBLOCK);

  for (int i = 0; i < sockets.size(); i++)
    epoll_ctl(sockets[i], EPOLL_CTL_ADD, EPOLLIN | EPOLLOUT | EPOLLET);

  /* connect: create a connection with the server */
  std::vector<bool> opened(sockets.size(), false);
  while (true) {
    for (int i = 0; i < sockets.size(); i++) {
      if (opened[i])
        continue;
      // int ret = connect(sockets[i], (const sockaddr*)serveraddr, sizeof(*serveraddr));
      int ret = connect(sockets[i], (const sockaddr*)&server, sizeof(server));
      if (ret == 0)
        opened[i] = true;
      else if (errno == EINPROGRESS || errno == EALREADY)
        continue;
      else
        http_benchmark_impl::error(std::string("Cannot connect to server: ") + strerror(errno));
    }

    int nopened = 0;
    for (int i = 0; i < sockets.size(); i++)
      nopened += opened[i];
    if (nopened == sockets.size())
      break;
  }

  close(epoll_fd);
  return sockets;
}

inline void http_benchmark_close(const std::vector<int>& sockets) {
  for (int i = 0; i < sockets.size(); i++)
    close(sockets[i]);
}

inline float http_benchmark(const std::vector<int>& sockets, int NTHREADS, int duration_in_ms,
                     std::string_view req) {

  int NCONNECTION_PER_THREAD = sockets.size() / NTHREADS;

  auto client_fn = [&](auto conn_handler, int i_start, int i_end) {
    int epoll_fd = epoll_create1(0);

    auto epoll_ctl = [epoll_fd](int fd, int op, uint32_t flags) {
      epoll_event event;
      event.data.fd = fd;
      event.events = flags;
      ::epoll_ctl(epoll_fd, op, fd, &event);
      return true;
    };

    for (int i = i_start; i < i_end; i++) {
      epoll_ctl(sockets[i], EPOLL_CTL_ADD, EPOLLIN | EPOLLOUT | EPOLLET);
    }

    const int MAXEVENTS = 64;
    std::vector<ctx::continuation> fibers;
    for (int i = i_start; i < i_end; i++) {
      int infd = sockets[i];
      if (int(fibers.size()) < infd + 1)
        fibers.resize(infd + 10);

      // std::cout << "start socket " << i << std::endl; 
      fibers[infd] = ctx::callcc([fd = infd, &conn_handler, epoll_ctl](ctx::continuation&& sink) {
        auto read = [fd, &sink, epoll_ctl](char* buf, int max_size) {
          ssize_t count = ::recv(fd, buf, max_size, 0);
          while (count <= 0) {
            if ((count < 0 and errno != EAGAIN) or count == 0)
              return ssize_t(0);
            sink = sink.resume();
            count = ::recv(fd, buf, max_size, 0);
          }
          return count;
        };

        auto write = [fd, &sink, epoll_ctl](const char* buf, int size) {
          const char* end = buf + size;
          ssize_t count = ::send(fd, buf, end - buf, 0);
          if (count > 0)
            buf += count;
          while (buf != end) {
            if ((count < 0 and errno != EAGAIN) or count == 0)
              return false;
            sink = sink.resume();
            count = ::send(fd, buf, end - buf, 0);
            if (count > 0)
              buf += count;
          }
          return true;
        };
        conn_handler(fd, read, write);
        return std::move(sink);
      });
    }

    // Even loop.
    epoll_event events[MAXEVENTS];
    timer global_timer;
    global_timer.start();
    global_timer.end();
    while (global_timer.ms() < duration_in_ms) {
      // std::cout << global_timer.ms() << " " << duration_in_ms << std::endl;
      int n_events = epoll_wait(epoll_fd, events, MAXEVENTS, 1);
      for (int i = 0; i < n_events; i++) {
        if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
        } else // Data available on existing sockets. Wake up the fiber associated with
               // events[i].data.fd.
          fibers[events[i].data.fd] = fibers[events[i].data.fd].resume();
      }
      global_timer.end();
    }
  };

  std::atomic<int> nmessages = 0;
  int pipeline_size = 50;

  auto bench_tcp = [&](int thread_id) {
    return [=, &nmessages]() {
      client_fn(
          [&](int fd, auto read, auto write) { // flood the server.
            std::string pipelined;
            for (int i = 0; i < pipeline_size; i++)
              pipelined += req;
            while (true) {
              char buf_read[10000];
              write(pipelined.data(), pipelined.size());
              int rd = read(buf_read, sizeof(buf_read));
              if (rd == 0) break;
              nmessages+=pipeline_size;
            }
          },
          thread_id * NCONNECTION_PER_THREAD, (thread_id+1) * NCONNECTION_PER_THREAD);
    };
  };

  timer global_timer;
  global_timer.start();

  int nthreads = NTHREADS;
  std::vector<std::thread> ths;
  for (int i = 0; i < nthreads; i++)
    ths.push_back(std::thread(bench_tcp(i)));
  for (auto& t : ths)
    t.join();

  global_timer.end();
  return (1000. * nmessages / global_timer.ms());
}

inline float http_benchmark_new_connections(int NTHREADS, int duration_in_ms, int port,
                                            std::string_view req) {
  auto parse_content_length = [](std::string_view headers) {
    const std::string key = "Content-Length:";
    size_t key_pos = headers.find(key);
    if (key_pos == std::string::npos)
      return size_t(0);

    size_t cur = key_pos + key.size();
    while (cur < headers.size() && headers[cur] == ' ')
      cur++;

    size_t end = cur;
    while (end < headers.size() && headers[end] >= '0' && headers[end] <= '9')
      end++;

    if (end == cur)
      return size_t(0);
    return size_t(std::strtoull(headers.substr(cur, end - cur).data(), nullptr, 10));
  };

  auto recv_one_http_response = [&](int fd) {
    std::string resp;
    resp.reserve(4096);

    char tmp[4096];
    while (true) {
      ssize_t rd = ::recv(fd, tmp, sizeof(tmp), 0);
      if (rd <= 0)
        return false;

      resp.append(tmp, rd);
      size_t header_end = resp.find("\r\n\r\n");
      if (header_end == std::string::npos)
        continue;

      size_t body_len = parse_content_length(std::string_view(resp.data(), header_end + 4));
      size_t expected_size = header_end + 4 + body_len;
      while (resp.size() < expected_size) {
        rd = ::recv(fd, tmp, sizeof(tmp), 0);
        if (rd <= 0)
          return false;
        resp.append(tmp, rd);
      }
      return true;
    }
  };

  std::atomic<int64_t> nmessages = 0;
  std::vector<std::thread> ths;
  ths.reserve(NTHREADS);

  const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(duration_in_ms);

  for (int t = 0; t < NTHREADS; t++) {
    ths.emplace_back([&, deadline] {
      sockaddr_in server;
      std::memset(&server, 0, sizeof(server));
      server.sin_addr.s_addr = inet_addr("127.0.0.1");
      server.sin_family = AF_INET;
      server.sin_port = htons(port);

      while (std::chrono::steady_clock::now() < deadline) {
        int fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0)
          continue;

        if (::connect(fd, (const sockaddr*)&server, sizeof(server)) != 0) {
          ::close(fd);
          continue;
        }

        const char* buf = req.data();
        size_t rem = req.size();
        bool ok = true;
        while (rem > 0) {
          ssize_t wr = ::send(fd, buf, rem, 0);
          if (wr <= 0) {
            ok = false;
            break;
          }
          buf += wr;
          rem -= wr;
        }

        if (ok && recv_one_http_response(fd))
          nmessages++;

        ::close(fd);
      }
    });
  }

  for (auto& th : ths)
    th.join();

  if (duration_in_ms <= 0)
    return 0.f;
  return (1000.f * float(nmessages.load()) / float(duration_in_ms));
}

} // namespace li
