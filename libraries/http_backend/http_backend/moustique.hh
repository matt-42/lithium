/**
 * @file   moustique.hh
 * @author Matthieu Garrigues <matthieu.garrigues@gmail.com> <matthieu.garrigues@gmail.com>
 * @date   Sat Mar 31 23:52:51 2018
 *
 * @brief  moustique wrapper.
 *
 *
 */
#pragma once

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <boost/context/continuation.hpp>
#include <thread>
#include <vector>

/**
 * Open a socket on port \port and call \conn_handler(int client_fd, auto read, auto write)
 * to process each incomming connection. This handle takes 3 argments:
 *               - int client_fd: the file descriptor of the socket.
 *               - int read(buf, max_size_to_read):
 *                       The callback that conn_handler can use to read on the socket.
 *                       If data is available, copy it into \buf, otherwise suspend the handler
 * until there is something to read. Returns the number of char actually read, returns 0 if the
 * connection has been lost.
 *               - bool write(buf, buf_size): return true on success, false on error.
 *                       The callback that conn_handler can use to write on the socket.
 *                       If the socket is ready to write, write \buf, otherwise suspend the handler
 * until it is ready. Returns true on sucess, false if connection is lost.
 *
 * @param port  The server port.
 * @param socktype The socket type, SOCK_STREAM for TCP, SOCK_DGRAM for UDP.
 * @param nthreads Number of threads.
 * @param conn_handler The connection handler
 * @return false on error, true on success.
 */
template <typename H> int moustique_listen(int port, int socktype, int nthreads, H conn_handler);

// Same as above but take an already opened socket \listen_fd.
template <typename H> int moustique_listen_fd(int listen_fd, int nthreads, H conn_handler);

namespace moustique_impl {
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

  return sfd;
}

} // namespace moustique_impl

#define MOUSTIQUE_CHECK_CALL(CALL)                                                                 \
  {                                                                                                \
    int ret = CALL;                                                                                \
    if (-1 == ret) {                                                                               \
      fprintf(stderr, "Error at %s:%i  error is: %s\n", __PRETTY_FUNCTION__, __LINE__,             \
              strerror(errno));                                                                    \
    }                                                                                              \
  }

template <typename H> int moustique_listen(int port, int socktype, int nthreads, H conn_handler) {
  return moustique_listen_fd(moustique_impl::create_and_bind(port, socktype), nthreads,
                             conn_handler);
}

static volatile int moustique_exit_request = 0;

static void shutdown_handler(int sig) {
  moustique_exit_request = 1;
  std::cout << "The server will shutdown..." << std::endl;
}

struct fiber_exception {

  std::string what;
  boost::context::continuation c;
  fiber_exception(fiber_exception&& e) : what{std::move(e.what)}, c{std::move(e.c)} {}
  fiber_exception(boost::context::continuation&& c_, std::string const& what)
      : what{what}, c{std::move(c_)} {}
};

using type_info_ref = std::reference_wrapper<const std::type_info>;

struct type_info_hash {
  std::size_t operator()(type_info_ref code) const { return code.get().hash_code(); }
};
struct type_info_equal {
  bool operator()(type_info_ref lhs, type_info_ref rhs) const { return lhs.get() == rhs.get(); }
};

void epoll_ctl(int epoll_fd, int fd, int action, uint32_t flags) {
  epoll_event event;
  memset(&event, 0, sizeof(event));
  event.data.fd = fd;
  event.events = flags;
  ::epoll_ctl(epoll_fd, action, fd, &event);
};

template <typename H> int moustique_listen_fd(int listen_fd, int nthreads, H conn_handler) {
  namespace ctx = boost::context;

  if (listen_fd < 0)
    return 0;
  int flags = fcntl(listen_fd, F_GETFL, 0);
  MOUSTIQUE_CHECK_CALL(fcntl(listen_fd, F_SETFL, flags | O_NONBLOCK));
  MOUSTIQUE_CHECK_CALL(::listen(listen_fd, SOMAXCONN));

  auto event_loop_fn = [listen_fd, conn_handler]() -> int {
    int epoll_fd = epoll_create1(0);

    epoll_ctl(epoll_fd, listen_fd, EPOLL_CTL_ADD, EPOLLIN | EPOLLET);

    const int MAXEVENTS = 64;
    std::vector<ctx::continuation> fibers;
    std::vector<int> fd_to_fiber_idx;

    auto fiber_from_fd = [&](int fd) -> ctx::continuation& { 
      assert(fd >= 0 and fd < fd_to_fiber_idx.size());
      return fibers[fd_to_fiber_idx[fd]]; };

    auto epoll_ctl_del = [&](int fd) {
      if (fd >= 0 and fd < fd_to_fiber_idx.size())
        fd_to_fiber_idx[fd] = -1;
      ::epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
      return true;
    };

    epoll_event events[MAXEVENTS];

    // // Non io events queue.
    // // For each event_type store the number of pending event and the waiting list
    // // of fiber file descriptors.
    // std::unordered_map<type_info_ref, std::pair<int, std::deque<int>>, type_info_hash,
    // type_info_equal> non_io_events;

    // // Mark a fiber file descriptor as waiting for a given event.
    // auto wait_for_non_io_event = [&] (auto event, int fd) {
    //   non_io_events[typeid(event)].second.push_back(fd);
    // };
    // auto notify_non_io_event = [&] (auto event) {
    //   non_io_events[typeid(event)].first++;
    // };

    // Even loop.
    while (!moustique_exit_request) {

      // // Process non_io_events.
      // for (auto pair : non_io_events)
      // {
      //   int& nevents = pair.second.first;
      //   std::deque<int>& waiting = pair.second.second;
      //   while (nevents > 0 and waiting.size() > 0)
      //   {
      //     int to_wakeup = waiting.front();
      //     waiting.pop_front();
      //     assert(to_wakeup < fibers.size());
      //     if (!fibers[to_wakeup]) continue;
      //     fibers[to_wakeup] = fibers[to_wakeup].resume();
      //     nevents--;
      //   }
      // }

      // std::cout << "before wait" << std::endl;
      int n_events = epoll_wait(epoll_fd, events, MAXEVENTS, 1);
      // std::cout << "end wait" << std::endl;
      if (moustique_exit_request)
        break;

      if (n_events == 0)
        for (int i = 0; i < fibers.size(); i++)
          if (fibers[i])
            fibers[i] = fibers[i].resume();
      
      for (int i = 0; i < n_events; i++)
      {
        if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) ||
            (events[i].events & EPOLLRDHUP)) {
          ctx::continuation& fiber = fiber_from_fd(events[i].data.fd);
          if (fiber)
            fiber = fiber.resume_with(std::move([](auto&& sink) {
              throw fiber_exception(std::move(sink), "EPOLLRDHUP");
              return std::move(sink);
            }));

          continue;
        } else if (listen_fd == events[i].data.fd) // New connection.
        {
          while (true) {
            struct sockaddr in_addr;
            socklen_t in_len;
            int infd;

            in_len = sizeof in_addr;
            infd = accept(listen_fd, &in_addr, &in_len);
            if (infd == -1)
              break;

            // Subscribe epoll to the socket file descriptor.
            if (-1 == fcntl(infd, F_SETFL, fcntl(infd, F_GETFL, 0) | O_NONBLOCK))
              continue;

            // Find a free fiber for this new connection.
            int fiber_idx = 0;
            while (fiber_idx < fibers.size() && fibers[fiber_idx])
              fiber_idx++;
            if (fiber_idx >= fibers.size())
              fibers.resize((fibers.size()+1) * 2);


            // Function to subscribe to other files descriptor.
            auto listen_to_new_fd = [epoll_fd, &fd_to_fiber_idx, fiber_idx](int new_fd, int flags = EPOLLET) {
              // Listen to the fd if not already done before.
              epoll_ctl(epoll_fd, new_fd, EPOLL_CTL_ADD, EPOLLIN | EPOLLOUT | EPOLLRDHUP | flags);
              // Associate new_fd to the fiber.
              if (int(fd_to_fiber_idx.size()) < new_fd + 1)
                fd_to_fiber_idx.resize((new_fd + 1) * 2, -1);
              fd_to_fiber_idx[new_fd] = fiber_idx;
            };

            listen_to_new_fd(infd, EPOLLET);

            auto terminate_fiber = [&](int fd) {
              if (fd < 0 or fd >= fd_to_fiber_idx.size()) {
                std::cerr << "terminate_fiber: Bad fd " << fd << std::endl;
                return;
              }
              if (0 != close(fd))
                std::cerr << "Error when closing file descriptor " << fd << ": " << strerror(errno)
                          << std::endl;
            };

            struct end_of_file {};
            assert(fiber_idx < fibers.size());
            fibers[fiber_idx] = ctx::callcc([fd = infd, &conn_handler, epoll_ctl_del, // wait_for_non_io_event,
                                        listen_to_new_fd, terminate_fiber](ctx::continuation&& sink) {
              try {

                // ctx::continuation sink = std::move(_sink);
                auto read = [fd, &sink](char* buf, int max_size) {
                  ssize_t count = ::recv(fd, buf, max_size, 0);
                  while (count <= 0) {
                    if ((count < 0 and errno != EAGAIN) or count == 0)
                      return ssize_t(0);
                    sink = sink.resume();
                    count = ::recv(fd, buf, max_size, 0);
                  }
                  return count;
                };

                auto write = [fd, &sink](const char* buf, int size) {
                  if (!buf or !size) {
                    // std::cout << "pause" << std::endl;
                    sink = sink.resume();
                    return true;
                  }
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
                // auto wait_for = [&](auto e) {
                //   wait_for_non_io_event(e, fd);
                //   sink = sink.resume();
                // };
                // FIXME MERGE:
                //   read, write, listen_to_new_fd, wait_for, notify in one object.
                conn_handler(fd, read, write, listen_to_new_fd, epoll_ctl_del);
                terminate_fiber(fd);
              } catch (fiber_exception& ex) {
                terminate_fiber(fd);
                return std::move(ex.c);
              } catch (const std::runtime_error& e) {
                terminate_fiber(fd);
                std::cerr << "FATAL ERRROR: exception in fiber: " << e.what() << std::endl;
                assert(0);
                return std::move(sink);
              }

              return std::move(sink);
            });
          }

        } else // Data available on existing sockets. Wake up the fiber associated with
              // events[i].data.fd.
        {
          int event_fd = events[i].data.fd;
          if (event_fd >= 0 && event_fd < fd_to_fiber_idx.size()) {
            auto& fiber = fiber_from_fd(event_fd);
            if(fiber) fiber = fiber.resume();
          }
          else
            std::cerr << "Epoll returned a file descriptor that we did not register: " << event_fd << std::endl;
        }
      }
    }
    std::cout << "END OF EVENT LOOP" << std::endl;
    close(epoll_fd);
    return true;
  };

  struct sigaction act;
  memset(&act, 0, sizeof(act));
  act.sa_handler = shutdown_handler;

  sigaction(SIGINT, &act, 0);
  sigaction(SIGTERM, &act, 0);
  sigaction(SIGQUIT, &act, 0);

  std::vector<std::thread> ths;
  for (int i = 0; i < nthreads; i++)
    ths.push_back(std::thread([&] { event_loop_fn(); }));

  for (auto& t : ths)
    t.join();

  close(listen_fd);

  return true;
}
