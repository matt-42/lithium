#pragma once

#include <iostream>

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/event.h> // Kqueue
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <deque>
#include <thread>
#include <vector>

#include <boost/context/continuation.hpp>

#include <li/http_backend/ssl_context.hh>

namespace li {

namespace impl {

// Helper to create a TCP/UDP server socket.
static int create_and_bind(int port, int socktype) {
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int s, sfd;

  char port_str[20];
  snprintf(port_str, sizeof(port_str), "%d", port);
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;  /* Return IPv4 and IPv6 choices */
  hints.ai_socktype = socktype; /* We want a TCP socket */
  hints.ai_flags = AI_PASSIVE;  /* All interfaces */

  s = getaddrinfo(NULL, port_str, &hints, &result);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    return -1;
  }

  for (rp = result; rp != NULL; rp = rp->ai_next) {
    sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sfd == -1)
      continue;

    s = bind(sfd, rp->ai_addr, rp->ai_addrlen);
    if (s == 0) {
      /* We managed to bind successfully! */
      break;
    }

    close(sfd);
  }

  if (rp == NULL) {
    fprintf(stderr, "Could not bind: %s\n", strerror(errno));
    return -1;
  }

  freeaddrinfo(result);

  int flags = fcntl(sfd, F_GETFL, 0);
  fcntl(sfd, F_SETFL, flags | O_NONBLOCK);
  ::listen(sfd, SOMAXCONN);

  return sfd;
}

} // namespace impl

static volatile int quit_signal_catched = 0;

struct async_fiber_context;

// Epoll based Reactor:
// Orchestrates a set of fiber (boost::context::continuation).

struct fiber_exception {

  std::string what;
  boost::context::continuation c;
  fiber_exception(fiber_exception&& e) = default;
  fiber_exception(boost::context::continuation&& c_, std::string const& what)
      : what{what}, c{std::move(c_)} {}
};

struct async_reactor;

// The fiber context passed to all fibers so they can do
//  yield, non blocking read/write on the socket fd, and subscribe to
//  other file descriptors events.
struct async_fiber_context {

  typedef fiber_exception exception_type;

  async_reactor* reactor;
  boost::context::continuation sink;
  int fiber_id;
  int socket_fd;
  sockaddr in_addr;
  SSL* ssl = nullptr;

  inline async_fiber_context& operator=(const async_fiber_context&) = delete;
  inline async_fiber_context(const async_fiber_context&) = delete;

  inline async_fiber_context(async_reactor* reactor, boost::context::continuation&& sink,
                      int fiber_id, int socket_fd, sockaddr in_addr)
      : reactor(reactor), sink(std::forward<boost::context::continuation&&>(sink)),
        fiber_id(fiber_id), socket_fd(socket_fd),
        in_addr(in_addr) {}

  inline void yield() { sink = sink.resume(); }
       
  inline bool ssl_handshake(std::unique_ptr<ssl_context>& ssl_ctx) {
    if (!ssl_ctx) return false;

    ssl = SSL_new(ssl_ctx->ctx);
    SSL_set_fd(ssl, socket_fd);

    while (int ret = SSL_accept(ssl)) {
      if (ret == 1) return true;

      int err = SSL_get_error(ssl, ret);
      if (err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_READ)
        this->yield();
      else
      {
        ERR_print_errors_fp(stderr);
        return false;
        // throw std::runtime_error("Error during https handshake.");
      }
    }
    return false;
  }

  inline ~async_fiber_context() {
    if (ssl)
    {
      SSL_shutdown(ssl);
      SSL_free(ssl);
    }
  }

  inline void wakeup_on_read_write_events(int fd);

  inline void epoll_add(int fd, int flags);
  inline void epoll_mod(int fd, int flags);
  inline void reassign_fd_to_this_fiber(int fd);

  inline void defer(const std::function<void()>& fun);
  inline void defer_fiber_resume(int fiber_id);

  inline int read_impl(char* buf, int size) {
    if (ssl)
      return SSL_read(ssl, buf, size);
    else
      return ::recv(socket_fd, buf, size, 0);
  }
  inline int write_impl(const char* buf, int size) {
    if (ssl)
      return SSL_write(ssl, buf, size);
    else
      return ::send(socket_fd, buf, size, 0);
  }

  inline int read(char* buf, int max_size) {
    ssize_t count = read_impl(buf, max_size);
    while (count <= 0) {
      if ((count < 0 and errno != EAGAIN) or count == 0)
        return ssize_t(0);
      sink = sink.resume();
      count = read_impl(buf, max_size);
    }
    return count;
  };

  inline bool write(const char* buf, int size) {
    if (!buf or !size) {
      // std::cout << "pause" << std::endl;
      sink = sink.resume();
      return true;
    }
    const char* end = buf + size;
    ssize_t count = write_impl(buf, end - buf);
    if (count > 0)
      buf += count;
    while (buf != end) {
      if ((count < 0 and errno != EAGAIN) or count == 0)
        return false;
      sink = sink.resume();
      count = write_impl(buf, end - buf);
      if (count > 0)
        buf += count;
    }
    return true;
  };
};

struct async_reactor {

  typedef boost::context::continuation continuation;

  int kqueue_fd;
  std::vector<continuation> fibers;
  std::vector<int> fd_to_fiber_idx;
  std::unique_ptr<ssl_context> ssl_ctx = nullptr;
  std::vector<std::function<void()>> defered_functions;
  std::deque<int> defered_resume;

  inline continuation& fd_to_fiber(int fd) {
    assert(fd >= 0 and fd < fd_to_fiber_idx.size());
    int fiber_idx = fd_to_fiber_idx[fd];
    assert(fiber_idx >= 0 and fiber_idx < fibers.size());

    return fibers[fiber_idx];
  }

  inline void reassign_fd_to_fiber(int fd, int fiber_idx) {
    fd_to_fiber_idx[fd] = fiber_idx;
  }

  // actions: EV_ADD
  inline void epoll_ctl(int kqueue_fd, int fd, int action, uint32_t flags) {
    struct kevent ev_set;
    EV_SET(&ev_set, fd, flags, action, 0, 0, NULL);
    kevent(kqueue_fd, &ev_set, 1, NULL, 0, NULL);
  };

  inline void epoll_add(int new_fd, int flags, int fiber_idx = -1) {
    epoll_ctl(kqueue_fd, new_fd, EV_ADD, flags);
    // Associate new_fd to the fiber.
    if (int(fd_to_fiber_idx.size()) < new_fd + 1)
      fd_to_fiber_idx.resize((new_fd + 1) * 2, -1);
    fd_to_fiber_idx[new_fd] = fiber_idx;
  }

  inline void epoll_mod(int fd, int flags) { epoll_ctl(kqueue_fd, fd, EV_ADD, flags); }

  template <typename H> void event_loop(int listen_fd, H handler) {

    this->kqueue_fd = kqueue();
    epoll_ctl(kqueue_fd, listen_fd, EV_ADD, EVFILT_READ);
    epoll_ctl(kqueue_fd, SIGINT, EV_ADD, EVFILT_SIGNAL);
    epoll_ctl(kqueue_fd, SIGKILL, EV_ADD, EVFILT_SIGNAL);
    epoll_ctl(kqueue_fd, SIGTERM, EV_ADD, EVFILT_SIGNAL);

    const int MAXEVENTS = 64;
    struct kevent events[MAXEVENTS];

    // Main loop.
    struct timespec timeout;
    memset(&timeout, 0, sizeof(timeout));
    timeout.tv_nsec = 10000;
    while (!quit_signal_catched) {

      // int n_events = epoll_wait(kqueue_fd, events, MAXEVENTS, 1);
      int n_events = kevent(kqueue_fd, NULL, 0, events, MAXEVENTS, &timeout);

      if (quit_signal_catched)
        break;

      if (n_events == 0 )
        for (int i = 0; i < fibers.size(); i++)
          if (fibers[i])
            fibers[i] = fibers[i].resume();

      for (int i = 0; i < n_events; i++) {

        // int event_flags = events[i].events;
        // int event_fd = events[i].data.fd;
        int event_flags = events[i].flags;
        int event_fd = events[i].ident;

        if (events[i].filter == EVFILT_SIGNAL && 
        (
          event_fd == SIGINT || event_fd == SIGTERM || event_fd == SIGKILL
        ))
        {
          if (event_fd == SIGINT) std::cout << "SIGINT" << std::endl; 
          if (event_fd == SIGTERM) std::cout << "SIGTERM" << std::endl; 
          if (event_fd == SIGKILL) std::cout << "SIGKILL" << std::endl; 
          quit_signal_catched = true;
          break;
        }
        // Handle error on sockets.
        // if (event_flags & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
        if (event_flags & EV_ERROR) {
          if (event_fd == listen_fd) {
            std::cout << "FATAL ERROR: Error on server socket " << event_fd << std::endl;
            quit_signal_catched = true;
          } else {
            continuation& fiber = fd_to_fiber(event_fd);
            if (fiber)
              fiber = fiber.resume_with(std::move([](auto&& sink) {
                throw fiber_exception(std::move(sink), "EPOLLRDHUP");
                return std::move(sink);
              }));
          }
        }
        // Handle new connections.
        else if (listen_fd == event_fd) {
          while (true) {

            // ============================================
            // ACCEPT INCOMMING CONNECTION
            struct sockaddr in_addr;
            socklen_t in_len;
            int socket_fd;
            in_len = sizeof in_addr;
            socket_fd = accept(listen_fd, &in_addr, &in_len);
            if (socket_fd == -1)
              break;
            // ============================================

            // ============================================
            // Find a free fiber for this new connection.
            int fiber_idx = 0;
            while (fiber_idx < fibers.size() && fibers[fiber_idx])
              fiber_idx++;
            if (fiber_idx >= fibers.size())
              fibers.resize((fibers.size() + 1) * 2);
            assert(fiber_idx < fibers.size());
            // ============================================

            // ============================================
            // Subscribe epoll to the socket file descriptor.
            if (-1 == fcntl(socket_fd, F_SETFL, fcntl(socket_fd, F_GETFL, 0) | O_NONBLOCK))
              continue;
            this->epoll_add(socket_fd, EVFILT_READ | EVFILT_WRITE, fiber_idx);
            // ============================================

            // ============================================
            // Simply utility to close fd at the end of a scope.
            struct scoped_fd {
              int fd;
              ~scoped_fd() {
                if (0 != close(fd))
                  std::cerr << "Error when closing file descriptor " << fd << ": "
                            << strerror(errno) << std::endl;
              }
            };
            // ============================================

            // =============================================
            // Spawn a new continuation to handle the connection.
            fibers[fiber_idx] = boost::context::callcc([this, socket_fd, fiber_idx, in_addr,
                                                        &handler](continuation&& sink) {
              scoped_fd sfd{socket_fd}; // Will finally close the fd.
              auto ctx = async_fiber_context(this, std::move(sink), fiber_idx, socket_fd, in_addr);
              try {
                if (ssl_ctx && !ctx.ssl_handshake(this->ssl_ctx))
                {
                  std::cerr << "Error during SSL handshake" << std::endl;
                  return std::move(ctx.sink);
                }
                handler(ctx);
              } catch (fiber_exception& ex) {
                return std::move(ex.c);
              } catch (const std::runtime_error& e) {
                std::cerr << "FATAL ERRROR: exception in fiber: " << e.what() << std::endl;
                assert(0);
                return std::move(ctx.sink);
              }
              return std::move(ctx.sink);
            });
            // =============================================
          }
        } else // Data available on existing sockets. Wake up the fiber associated with
               // event_fd.
        {
          if (event_fd >= 0 && event_fd < fd_to_fiber_idx.size()) {
            auto& fiber = fd_to_fiber(event_fd);
            if (fiber)
              fiber = fiber.resume();
          } else
            std::cerr << "Epoll returned a file descriptor that we did not register: " << event_fd
                      << std::endl;
        }

        // Wakeup fibers if needed.
        while (defered_resume.size())
        {
          int fiber_id = defered_resume.front();
          defered_resume.pop_front();
          assert(fiber_id < fibers.size());
          auto& fiber = fibers[fiber_id];
          if (fiber)
          {
            // std::cout << "wakeup " << fiber_id << std::endl; 
            fiber = fiber.resume();
          }
        }


      }

      if (quit_signal_catched)
        break;

      // Call and Flush the defered functions.
      if (defered_functions.size())
      {
        for (auto& f : defered_functions)
          f();
        defered_functions.clear();
      }

    }
    std::cout << "END OF EVENT LOOP" << std::endl;
    // close(kqueue_fd); no need to close on kqueue ?
  }
};

static void shutdown_handler(int sig) {
  quit_signal_catched = 1;
  std::cout << "The server will shutdown..." << std::endl;
}

void async_fiber_context::epoll_add(int fd, int flags) {
  reactor->epoll_add(fd, flags, fiber_id);
}
void async_fiber_context::epoll_mod(int fd, int flags) { reactor->epoll_mod(fd, flags); }

void async_fiber_context::wakeup_on_read_write_events(int fd) {
  reactor->epoll_mod(fd, EVFILT_READ | EVFILT_WRITE);
}

void async_fiber_context::reassign_fd_to_this_fiber(int fd) {
  this->reactor->reassign_fd_to_fiber(fd, this->fiber_id);
}

void async_fiber_context::defer(const std::function<void()>& fun)
{
  this->reactor->defered_functions.push_back(fun);
}

void async_fiber_context::defer_fiber_resume(int fiber_id)
{
  this->reactor->defered_resume.push_back(fiber_id);
}

template <typename H>
void start_tcp_server(int port, int socktype, int nthreads, H conn_handler,
                      std::string ssl_key_path = "", std::string ssl_cert_path = "",
                      std::string ssl_ciphers = "") {

  struct sigaction act;
  memset(&act, 0, sizeof(act));
  act.sa_handler = shutdown_handler;

  sigaction(SIGINT, &act, 0);
  sigaction(SIGTERM, &act, 0);
  sigaction(SIGQUIT, &act, 0);

  int server_fd = impl::create_and_bind(port, socktype);
  std::vector<std::thread> ths;
  for (int i = 0; i < nthreads; i++)
    ths.push_back(std::thread([&] {
      async_reactor reactor;
      if (ssl_cert_path.size()) // Initialize the SSL/TLS context.
        reactor.ssl_ctx = std::make_unique<ssl_context>(ssl_key_path, ssl_cert_path, ssl_ciphers);
      reactor.event_loop(server_fd, conn_handler);
    }));

  for (auto& t : ths)
    t.join();

  close(server_fd);
}

} // namespace li
