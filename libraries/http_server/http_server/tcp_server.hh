#pragma once

#include <iostream>

#include <errno.h>
#include <fcntl.h>
#if not defined(_WIN32)
#include <netdb.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#endif

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if __linux__
#include <sys/epoll.h>
#elif __APPLE__
#include <sys/event.h>
#endif

#if defined __linux__ || defined __APPLE__
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#if defined _WIN32
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <wepoll.h>
#endif

#include <deque>
#include <thread>
#include <vector>

#include <boost/context/continuation.hpp>

#include <li/http_server/ssl_context.hh>

namespace li {

namespace impl {

#if defined _WIN32
typedef SOCKET socket_type;
#else
typedef int socket_type;
#endif

inline int close_socket(socket_type sock) {
#if defined _WIN32
  return closesocket(sock);
#else
  return close(sock);
#endif
}

// Helper to create a TCP/UDP server socket.
static socket_type create_and_bind(const char* ip, int port, int socktype) {
  int s;
  socket_type sfd;

  if (ip == nullptr) {
    // No IP address was specified, find an appropriate one
    struct addrinfo hints;
    struct addrinfo *result, *rp;

    char port_str[20];
    snprintf(port_str, sizeof(port_str), "%d", port);
    memset(&hints, 0, sizeof(struct addrinfo));
    
    // On windows, setting up the dual-stack mode (ipv4/ipv6 on the same socket).
    // https://docs.microsoft.com/en-us/windows/win32/winsock/dual-stack-sockets
    hints.ai_family = AF_INET6;
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

      // Turn of IPV6_V6ONLY to accept ipv4.
      // https://stackoverflow.com/questions/1618240/how-to-support-both-ipv4-and-ipv6-connections
      int ipv6only = 0;
      if(setsockopt(sfd, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&ipv6only, sizeof(ipv6only)) != 0)
      {
        std::cerr << "FATAL ERROR: setsockopt error when setting IPV6_V6ONLY to 0: " << strerror(errno)
                  << std::endl;
      }

      s = bind(sfd, rp->ai_addr, int(rp->ai_addrlen));
      if (s == 0) {
        /* We managed to bind successfully! */
        break;
      }
      else {
        close_socket(sfd);
      }
    }

    if (rp == NULL) {
      fprintf(stderr, "Could not bind: %s\n", strerror(errno));
      return -1;
    }

    freeaddrinfo(result);
  } else {
    sfd = socket(AF_INET, socktype, 0);

    // Use the user specified IP address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = port;

    s = bind(sfd, (struct sockaddr *)&addr, sizeof(addr));
    if (s != 0) {
      fprintf(stderr, "Could not bind: %s\n", strerror(errno));
      close_socket(sfd);
      return -1;
    }
  }

#if _WIN32
  u_long set_on = 1;
  auto ret = ioctlsocket(sfd, (long)FIONBIO, &set_on);
  if (ret) {
    std::cerr << "FATAL ERROR: Cannot set socket to non blocking mode with ioctlsocket"
              << std::endl;
  }

#else
  int flags = fcntl(sfd, F_GETFL, 0);
  fcntl(sfd, F_SETFL, flags | O_NONBLOCK);

#endif
  
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
        fiber_id(fiber_id), socket_fd(socket_fd), in_addr(in_addr) {}

  inline void yield() { sink = sink.resume(); }

  inline bool ssl_handshake(std::unique_ptr<ssl_context>& ssl_ctx) {
    if (!ssl_ctx)
      return false;

    ssl = SSL_new(ssl_ctx->ctx);
    SSL_set_fd(ssl, socket_fd);

    while (int ret = SSL_accept(ssl)) {
      if (ret == 1)
        return true;

      int err = SSL_get_error(ssl, ret);
      if (err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_READ)
        this->yield();
      else {
        ERR_print_errors_fp(stderr);
        return false;
        // throw std::runtime_error("Error during https handshake.");
      }
    }
    return false;
  }

  inline ~async_fiber_context() {
    if (ssl) {
      SSL_shutdown(ssl);
      SSL_free(ssl);
    }
  }

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
    int count = read_impl(buf, max_size);
    while (count <= 0) {
      if ((count < 0 and errno != EAGAIN) or count == 0)
        return int(0);
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
    int count = write_impl(buf, end - buf);
    if (count > 0)
      buf += count;
    while (buf != end) {
      if ((count < 0 and errno != EAGAIN) or count == 0)
        return false;
      sink = sink.resume();
      count = write_impl(buf, int(end - buf));
      if (count > 0)
        buf += count;
    }
    return true;
  };
};

struct async_reactor {

  typedef boost::context::continuation continuation;

#if defined _WIN32
  typedef HANDLE epoll_handle_t;
  u_long iMode = 0;
#else
  typedef int epoll_handle_t;
#endif

  epoll_handle_t epoll_fd;
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

  inline void reassign_fd_to_fiber(int fd, int fiber_idx) { fd_to_fiber_idx[fd] = fiber_idx; }

  inline void epoll_ctl(epoll_handle_t epoll_fd, int fd, int action, uint32_t flags) {

#if __linux__ || _WIN32
    {
      epoll_event event;
      memset(&event, 0, sizeof(event));
      event.data.fd = fd;
      event.events = flags;
      if (-1 == ::epoll_ctl(epoll_fd, action, fd, &event) and errno != EEXIST)
        std::cout << "epoll_ctl error: " << strerror(errno) << std::endl;
    }
#elif __APPLE__
    {
      struct kevent ev_set;
      EV_SET(&ev_set, fd, flags, action, 0, 0, NULL);
      kevent(epoll_fd, &ev_set, 1, NULL, 0, NULL);
    }
#endif
  };

  inline void epoll_add(int new_fd, int flags, int fiber_idx = -1) {
#if __linux__ || _WIN32
    epoll_ctl(epoll_fd, new_fd, EPOLL_CTL_ADD, flags);
#elif __APPLE__
    epoll_ctl(epoll_fd, new_fd, EV_ADD, flags);
#endif

    // Associate new_fd to the fiber.
    if (int(fd_to_fiber_idx.size()) < new_fd + 1)
      fd_to_fiber_idx.resize((new_fd + 1) * 2, -1);
    fd_to_fiber_idx[new_fd] = fiber_idx;
  }

  inline void epoll_del(int fd) {
#if __linux__ || _WIN32
    epoll_ctl(epoll_fd, fd, EPOLL_CTL_DEL, 0);
#elif __APPLE__
    epoll_ctl(epoll_fd, fd, EV_DELETE, 0);
#endif
  }

  inline void epoll_mod(int fd, int flags) {
#if __linux__ || _WIN32
    epoll_ctl(epoll_fd, fd, EPOLL_CTL_MOD, flags);
#elif __APPLE__
    epoll_ctl(epoll_fd, fd, EV_ADD, flags);
#endif
  }

  template <typename H> void event_loop(int listen_fd, H handler) {

    const int MAXEVENTS = 64;

#if __linux__ || _WIN32
    this->epoll_fd = epoll_create1(0);
#ifdef _WIN32
    // epollet is not implemented by wepoll.
    epoll_ctl(epoll_fd, listen_fd, EPOLL_CTL_ADD, EPOLLIN);
#else
    epoll_ctl(epoll_fd, listen_fd, EPOLL_CTL_ADD, EPOLLIN | EPOLLET);
#endif

    epoll_event events[MAXEVENTS];

#elif __APPLE__
    this->epoll_fd = kqueue();
    epoll_ctl(this->epoll_fd, listen_fd, EV_ADD, EVFILT_READ);
    epoll_ctl(this->epoll_fd, SIGINT, EV_ADD, EVFILT_SIGNAL);
    epoll_ctl(this->epoll_fd, SIGKILL, EV_ADD, EVFILT_SIGNAL);
    epoll_ctl(this->epoll_fd, SIGTERM, EV_ADD, EVFILT_SIGNAL);
    struct kevent events[MAXEVENTS];

    struct timespec timeout;
    memset(&timeout, 0, sizeof(timeout));
    timeout.tv_nsec = 10000;
#endif

    // Main loop.
    while (!quit_signal_catched) {

#if __linux__ || _WIN32
      // Wakeup to check if any quit signal has been catched.
      int n_events = epoll_wait(epoll_fd, events, MAXEVENTS, 1);
      // std::cout << "got " << n_events << " epoll events." << std::endl;
#elif __APPLE__
      // kevent is already listening to quit signals.
      int n_events = kevent(epoll_fd, NULL, 0, events, MAXEVENTS, &timeout);
#endif

      if (quit_signal_catched)
        break;

      if (n_events == 0)
        for (int i = 0; i < fibers.size(); i++)
          if (fibers[i])
            fibers[i] = fibers[i].resume();

      for (int i = 0; i < n_events; i++) {

#if __APPLE__
        int event_flags = events[i].flags;
        int event_fd = events[i].ident;

        if (events[i].filter == EVFILT_SIGNAL) {
          if (event_fd == SIGINT)
            std::cout << "SIGINT" << std::endl;
          if (event_fd == SIGTERM)
            std::cout << "SIGTERM" << std::endl;
          if (event_fd == SIGKILL)
            std::cout << "SIGKILL" << std::endl;
          quit_signal_catched = true;
          break;
        }

#elif __linux__ || _WIN32
        int event_flags = events[i].events;
        int event_fd = events[i].data.fd;
#endif

        // Handle errors on sockets.
#if __linux__ || _WIN32
        if (event_flags & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
#elif __APPLE__
        if (event_flags & EV_ERROR) {
#endif
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
#ifndef _WIN32
          while (true) {
#endif
            // ============================================
            // ACCEPT INCOMMING CONNECTION
            sockaddr_storage in_addr_storage;
            sockaddr* in_addr = (sockaddr*) &in_addr_storage;
            socklen_t in_len;
            int socket_fd;
            in_len = sizeof(sockaddr_storage);
            socket_fd = accept(listen_fd, in_addr, &in_len);
            //socket_fd = accept(listen_fd, nullptr, nullptr);
            if (socket_fd == -1)
            {
              // if (errno != EAGAIN && errno != EWOULDBLOCK) {
              //   std::cerr << "accept error: " << strerror(errno) << std::endl;
              //   std::cerr << "accept error: " << WSAGetLastError() << std::endl;
              // }
              break;
            }   
            // ============================================

            // ============================================
            // Subscribe epoll to the socket file descriptor.
#if _WIN32
            if (ioctlsocket(socket_fd, FIONBIO, &iMode) != NO_ERROR) continue;
#else
            if (-1 == ::fcntl(socket_fd, F_SETFL, fcntl(socket_fd, F_GETFL, 0) | O_NONBLOCK)) continue;
#endif
            // ============================================
            // Find a free fiber for this new connection.
            int fiber_idx = 0;
            while (fiber_idx < fibers.size() && fibers[fiber_idx])
              fiber_idx++;
            if (fiber_idx >= fibers.size())
              fibers.resize((fibers.size() + 1) * 2);
            assert(fiber_idx < fibers.size());
            // ============================================
#if __linux__
            this->epoll_add(socket_fd, EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET, fiber_idx);
#elif _WIN32
            this->epoll_add(socket_fd, EPOLLIN | EPOLLOUT | EPOLLRDHUP, fiber_idx);
#elif __APPLE__
          this->epoll_add(socket_fd, EVFILT_READ | EVFILT_WRITE, fiber_idx);
#endif

            // ============================================

            // ============================================
            // Simply utility to close fd at the end of a scope.
            struct scoped_fd {
              impl::socket_type fd;
              ~scoped_fd() {
                if (0 != impl::close_socket(fd))
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
              auto ctx = async_fiber_context(this, std::move(sink), fiber_idx, socket_fd, *in_addr);
              try {
                if (ssl_ctx && !ctx.ssl_handshake(this->ssl_ctx)) {
                  std::cerr << "Error during SSL handshake" << std::endl;
                  return std::move(ctx.sink);
                }
                handler(ctx);
                epoll_del(socket_fd);
              } catch (fiber_exception& ex) {
                epoll_del(socket_fd);
                return std::move(ex.c);
              } catch (const std::runtime_error& e) {
                epoll_del(socket_fd);
                std::cerr << "FATAL ERRROR: exception in fiber: " << e.what() << std::endl;
                assert(0);
                return std::move(ctx.sink);
              }
              return std::move(ctx.sink);
            });
            // =============================================
#ifndef _WIN32
		  }
#endif
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
        while (defered_resume.size()) {
          int fiber_id = defered_resume.front();
          defered_resume.pop_front();
          assert(fiber_id < fibers.size());
          auto& fiber = fibers[fiber_id];
          if (fiber) {
            // std::cout << "wakeup " << fiber_id << std::endl;
            fiber = fiber.resume();
          }
        }
      }

      // Call and Flush the defered functions.
      if (defered_functions.size()) {
        for (auto& f : defered_functions)
          f();
        defered_functions.clear();
      }
    }
    std::cout << "END OF EVENT LOOP" << std::endl;
#if _WIN32
    epoll_close(epoll_fd);
#else
    impl::close_socket(epoll_fd);
#endif
  }
};

static void shutdown_handler(int sig) {
  quit_signal_catched = 1;
  std::cout << "The server will shutdown..." << std::endl;
}

void async_fiber_context::epoll_add(int fd, int flags) { reactor->epoll_add(fd, flags, fiber_id); }
void async_fiber_context::epoll_mod(int fd, int flags) { reactor->epoll_mod(fd, flags); }

void async_fiber_context::defer(const std::function<void()>& fun) {
  this->reactor->defered_functions.push_back(fun);
}

void async_fiber_context::defer_fiber_resume(int fiber_id) {
  this->reactor->defered_resume.push_back(fiber_id);
}

void async_fiber_context::reassign_fd_to_this_fiber(int fd) {
  this->reactor->reassign_fd_to_fiber(fd, this->fiber_id);
}

template <typename H>
void start_tcp_server(std::string ip, int port, int socktype, int nthreads, H conn_handler,
                      std::string ssl_key_path = "", std::string ssl_cert_path = "",
                      std::string ssl_ciphers = "") {

// Start the winsock DLL
#ifdef _WIN32
  WORD wVersionRequested;
  WSADATA wsaData;
  int err;
  wVersionRequested = MAKEWORD(2, 2);
  err = WSAStartup(wVersionRequested, &wsaData);
  if (err != 0) {
    std::cerr << "WSAStartup failed with error: " << err << std::endl;
    return;
  }
#endif

// Setup quit signals
#ifdef _WIN32
  signal(SIGINT, shutdown_handler);
  signal(SIGTERM, shutdown_handler);
  signal(SIGABRT, shutdown_handler);
#else
  struct sigaction act;
  memset(&act, 0, sizeof(act));
  act.sa_handler = shutdown_handler;
  sigaction(SIGINT, &act, 0);
  sigaction(SIGTERM, &act, 0);
  sigaction(SIGQUIT, &act, 0);
  // Ignore sigpipe signal. Otherwise sendfile causes crashes if the
  // client closes the connection during the response transfer.
  signal(SIGPIPE, SIG_IGN);
#endif

  // Start the server threads.
  const char *listen_ip = !ip.empty() ? ip.c_str() : nullptr;
  int server_fd = impl::create_and_bind(listen_ip, port, socktype);
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

  impl::close_socket(server_fd);
}

} // namespace li
