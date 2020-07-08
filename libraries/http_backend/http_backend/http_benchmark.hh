#pragma once

#include <atomic>
#include <boost/context/continuation.hpp>
#include <chrono>
#include <iostream>
#include <thread>

namespace ctx = boost::context;

namespace li {

namespace http_benchmark_impl {

class timer {
public:
  void start() { start_ = std::chrono::high_resolution_clock::now(); }
  void end() { end_ = std::chrono::high_resolution_clock::now(); }

  unsigned long us() const {
    return std::chrono::duration_cast<std::chrono::microseconds>(end_ - start_).count();
  }

  unsigned long ms() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(end_ - start_).count();
  }

  unsigned long ns() const {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end_ - start_).count();
  }

private:
  std::chrono::time_point<std::chrono::high_resolution_clock> start_, end_;
};

void error(std::string msg) {
  perror(msg.c_str());
  exit(0);
}

} // namespace http_benchmark_impl

std::vector<int> http_benchmark_connect(int NCONNECTIONS, int port) {
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

void http_benchmark_close(const std::vector<int>& sockets) {
  for (int i = 0; i < sockets.size(); i++)
    close(sockets[i]);
}

float http_benchmark(const std::vector<int>& sockets, int NTHREADS, int duration_in_ms,
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
    http_benchmark_impl::timer global_timer;
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

  auto bench_tcp = [&](int thread_id) {
    return [=, &nmessages]() {
      client_fn(
          [&](int fd, auto read, auto write) { // flood the server.
            while (true) {
              char buf_read[10000];
              write(req.data(), req.size());
              int rd = read(buf_read, sizeof(buf_read));
              nmessages++;
            }
          },
          thread_id * NCONNECTION_PER_THREAD, (thread_id+1) * NCONNECTION_PER_THREAD);
    };
  };

  int nthreads = NTHREADS;
  std::vector<std::thread> ths;
  for (int i = 0; i < nthreads; i++)
    ths.push_back(std::thread(bench_tcp(i)));
  for (auto& t : ths)
    t.join();
  return (1000. * nmessages / duration_in_ms);
}

} // namespace li
