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

float http_benchmark(int NCONNECTIONS, int NTHREADS, int duration_in_ms, int port,
                     std::string_view req) {
  auto client_fn = [&](auto conn_handler) {
    int sockets[NCONNECTIONS];
    int portno, n;
    struct sockaddr_in serveraddr;
    struct hostent* server;
    const char* hostname;

    /* check command line arguments */
    hostname = "0.0.0.0";
    portno = port;

    /* socket: create the socket */
    for (int i = 0; i < NCONNECTIONS; i++) {
      sockets[i] = socket(AF_INET, SOCK_STREAM, 0);
      if (sockets[i] < 0)
        http_benchmark_impl::error("ERROR opening socket");
    }

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
      fprintf(stderr, "ERROR, no such host as %s\n", hostname);
      exit(0);
    }

    /* build the server's Internet address */
    bzero((char*)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char*)server->h_addr, (char*)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);



    int epoll_fd = epoll_create1(0);

    auto epoll_ctl = [epoll_fd](int fd, int op, uint32_t flags) {
      epoll_event event;
      event.data.fd = fd;
      event.events = flags;
      ::epoll_ctl(epoll_fd, op, fd, &event);
      return true;
    };

    // Set sockets non blocking.
    for (int i = 0; i < NCONNECTIONS; i++)
      fcntl(sockets[i], F_SETFL, fcntl(sockets[i], F_GETFL, 0) | O_NONBLOCK);

    for (int i = 0; i < NCONNECTIONS; i++)
      epoll_ctl(sockets[i], EPOLL_CTL_ADD, EPOLLIN | EPOLLOUT | EPOLLET);

    /* connect: create a connection with the server */
    std::vector<bool> opened(NCONNECTIONS, false);
    while (true)
    {
      for (int i = 0; i < NCONNECTIONS; i++)
      {
        if (opened[i]) continue;
        int ret = connect(sockets[i], (const sockaddr*)&serveraddr, sizeof(serveraddr));
        if (ret == 0)
          opened[i] = true;
        else if (errno == EINPROGRESS || errno == EALREADY)
          continue;
        else
          http_benchmark_impl::error(std::string("Cannot connect to server: ") + strerror(errno));
      }

      int nopened = 0;
      for (int i = 0; i < NCONNECTIONS; i++)
        nopened += opened[i];
      if (nopened == NCONNECTIONS)
        break;
    }

    const int MAXEVENTS = 64;
    std::vector<ctx::continuation> fibers;
    for (int i = 0; i < NCONNECTIONS; i++) {
      int infd = sockets[i];
      if (int(fibers.size()) < infd + 1)
        fibers.resize(infd + 10);

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
          ssize_t count = ::send(fd, buf, end - buf, MSG_NOSIGNAL);
          if (count > 0)
            buf += count;
          while (buf != end) {
            if ((count < 0 and errno != EAGAIN) or count == 0)
              return false;
            sink = sink.resume();
            count = ::send(fd, buf, end - buf, MSG_NOSIGNAL);
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
      int n_events = epoll_wait(epoll_fd, events, MAXEVENTS, 1);
      for (int i = 0; i < n_events; i++) {
        if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
          close(events[i].data.fd);
          fibers[events[i].data.fd] = fibers[events[i].data.fd].resume();
        } else // Data available on existing sockets. Wake up the fiber associated with
               // events[i].data.fd.
          fibers[events[i].data.fd] = fibers[events[i].data.fd].resume();
      }

      global_timer.end();
    }

    // Close all client sockets.
    for (int i = 0; i < NCONNECTIONS; i++)
      close(sockets[i]);
  };

  std::atomic<int> nmessages = 0;

  auto bench_tcp = [&]() {
    client_fn([&](int fd, auto read, auto write) { // flood the server.
      while (true) {
        char buf_read[10000];
        write(req.data(), req.size());
        int rd = read(buf_read, sizeof(buf_read));
        nmessages++;
      }
    });
  };

  int nthreads = NTHREADS;
  std::vector<std::thread> ths;
  for (int i = 0; i < nthreads; i++)
    ths.push_back(std::thread(bench_tcp));
  for (auto& t : ths)
    t.join();
  return (1000. * nmessages / duration_in_ms);
}

} // namespace li