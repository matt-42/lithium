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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <signal.h>

#include <thread>
#include <vector>
#include <boost/context/continuation.hpp>

/** 
 * Open a socket on port \port and call \conn_handler(int client_fd, auto read, auto write) 
 * to process each incomming connection. This handle takes 3 argments:
 *               - int client_fd: the file descriptor of the socket.
 *               - int read(buf, max_size_to_read):  
 *                       The callback that conn_handler can use to read on the socket.
 *                       If data is available, copy it into \buf, otherwise suspend the handler until
 *                       there is something to read.
 *                       Returns the number of char actually read, returns 0 if the connection has been lost.
 *               - bool write(buf, buf_size): return true on success, false on error.
 *                       The callback that conn_handler can use to write on the socket.
 *                       If the socket is ready to write, write \buf, otherwise suspend the handler until
 *                       it is ready.
 *                       Returns true on sucess, false if connection is lost.
 *
 * @param port  The server port.
 * @param socktype The socket type, SOCK_STREAM for TCP, SOCK_DGRAM for UDP.
 * @param nthreads Number of threads.
 * @param conn_handler The connection handler
 * @return false on error, true on success.
 */
template <typename H>
int moustique_listen(int port,
                     int socktype,
                     int nthreads,
                     H conn_handler);

// Same as above but take an already opened socket \listen_fd.
template <typename H>
int moustique_listen_fd(int listen_fd,
                        int nthreads,
                        H conn_handler);

namespace moustique_impl
{
  static int create_and_bind(int port, int socktype)
  {
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s, sfd;

    char port_str[20];
    snprintf(port_str, sizeof(port_str), "%d", port);
    memset (&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
    hints.ai_socktype = socktype; /* We want a TCP socket */
    hints.ai_flags = AI_PASSIVE;     /* All interfaces */

    s = getaddrinfo (NULL, port_str, &hints, &result);
    if (s != 0)
    {
      fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (s));
      return -1;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
      sfd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
      if (sfd == -1)
        continue;

      s = bind (sfd, rp->ai_addr, rp->ai_addrlen);
      if (s == 0)
      {
        /* We managed to bind successfully! */
        break;
      }

      close (sfd);
    }

    if (rp == NULL)
    {
      fprintf (stderr, "Could not bind: %s\n", strerror(errno));
      return -1;
    }

    freeaddrinfo (result);

    return sfd;
  }

}

#define MOUSTIQUE_CHECK_CALL(CALL)                                      \
  {                                                                     \
    int ret = CALL;                                                     \
    if (-1 == ret)                                                      \
    {                                                                   \
      fprintf(stderr, "Error at %s:%i  error is: %s\n", __PRETTY_FUNCTION__, __LINE__, strerror(ret)); \
      return false;                                                     \
    }                                                                   \
  }

template <typename H>
int moustique_listen(int port,
                     int socktype,
                     int nthreads,
                     H conn_handler)
{
  return moustique_listen_fd(moustique_impl::create_and_bind(port, socktype), nthreads, conn_handler);
}

static volatile int moustique_exit_request = 0;

static void shutdown_handler(int sig) {
  moustique_exit_request = 1;
  std::cout << "The server will shutdown..." << std::endl;
}

struct fiber_exception {


    std::string what;
    boost::context::continuation    c;
    fiber_exception(fiber_exception&& e) : what{std::move(e.what)}, c{std::move(e.c)} {}
    fiber_exception(boost::context::continuation && c_,std::string const& what) :
        what { what },
        c{ std::move( c_) } {
    }
};


template <typename H>
int moustique_listen_fd(int listen_fd,
                        int nthreads,
                        H conn_handler)
{
  namespace ctx = boost::context;

  if (listen_fd < 0) return 0;
  int flags = fcntl (listen_fd, F_GETFL, 0);
  MOUSTIQUE_CHECK_CALL(fcntl(listen_fd, F_SETFL, flags | O_NONBLOCK));
  MOUSTIQUE_CHECK_CALL(::listen(listen_fd, SOMAXCONN));

  auto event_loop_fn = [listen_fd, conn_handler] () -> int {

    int epoll_fd = epoll_create1(0);

    auto epoll_ctl = [epoll_fd] (int fd, uint32_t flags)
    {
      epoll_event event;
      memset(&event, 0, sizeof(event));
      event.data.fd = fd;
      event.events = flags;
      MOUSTIQUE_CHECK_CALL(::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event));
      return true;
    };
    auto epoll_ctl_mod = [epoll_fd] (int fd, uint32_t flags)
    {
      epoll_event event;
      event.data.fd = fd;
      event.events = flags;
      MOUSTIQUE_CHECK_CALL(::epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event));
      return true;
    };
    auto epoll_ctl_del = [epoll_fd] (int fd)
    { 
      MOUSTIQUE_CHECK_CALL(::epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr));
      return true;
    };
    
    epoll_ctl(listen_fd, EPOLLIN | EPOLLET);

    const int MAXEVENTS = 64;
    std::vector<ctx::continuation> fibers;
    std::vector<int> secondary_map;
    // fibers.reserve(1000);
    // Even loop.
    epoll_event events[MAXEVENTS];
    //int nrunning = 0;
    std::vector<int> is_running;

    while (!moustique_exit_request)
    {

      //std::cout << "before wait" << std::endl;
      int n_events = epoll_wait (epoll_fd, events, MAXEVENTS, 100);
      //std::cout << "end wait" << std::endl;
      if (moustique_exit_request) 
        break;

      if (n_events == 0)
      {
        //int cpt = 0;
        for (int i = 0; i < fibers.size(); i++)
          if (is_running[i])
          {
            //cpt++;
            fibers[i] = fibers[i].resume();
          }
        //std::cout << "count : " << cpt << std::endl;;
      }

      for (int i = 0; i < n_events; i++)
      {

        if ((events[i].events & EPOLLERR) ||
            (events[i].events & EPOLLHUP) ||
            (events[i].events & EPOLLRDHUP)
            )
        {
          close (events[i].data.fd);
          //std::cout << "socket closed" << std::endl;
          if (is_running[events[i].data.fd])

            fibers[events[i].data.fd] = fibers[events[i].data.fd].resume_with(std::move([] (auto&& sink)  { 
              //std::cout << "throw socket closed" << std::endl;
              throw fiber_exception(std::move(sink), "Socket closed"); 
              return std::move(sink);
            }));

          //nrunning--;
          continue;
        }
        else if (listen_fd == events[i].data.fd) // New connection.
        {
          while(true)
          {
            //std::cout << "new connection" << std::endl;
            struct sockaddr in_addr;
            socklen_t in_len;
            int infd;

            in_len = sizeof in_addr;
            infd = accept (listen_fd, &in_addr, &in_len);
            if (infd == -1)
              break;


            // Subscribe epoll to the socket file descriptor.
            MOUSTIQUE_CHECK_CALL(fcntl(infd, F_SETFL, fcntl(infd, F_GETFL, 0) | O_NONBLOCK));
            epoll_ctl(infd, EPOLLIN | EPOLLOUT | EPOLLET | EPOLLRDHUP);

            // Function to subscribe to other files descriptor.
            auto listen_to_new_fd = [original_fd=infd,epoll_ctl,&secondary_map] (int new_fd) {
              if (new_fd > secondary_map.size() || secondary_map[new_fd] == -1)
                epoll_ctl(new_fd, EPOLLIN | EPOLLOUT | EPOLLET);
              if (int(secondary_map.size()) < new_fd + 1)
                secondary_map.resize(new_fd + 10, -1);             
              secondary_map[new_fd] = original_fd;
            };

            if (int(fibers.size()) < infd + 1)
            {
              fibers.resize(infd + 10);
              is_running.resize(infd + 10, false);
            }
            struct end_of_file {};
            fibers[infd] = ctx::callcc([fd=infd, &conn_handler, epoll_ctl_del, listen_to_new_fd, &is_running]
                                       (ctx::continuation&& sink) {
                                         try {
                                        //nrunning++;
                                        is_running[fd] = true;
                                        //std::cout << "nrunning: " << nrunning << std::endl;

                                         //ctx::continuation sink = std::move(_sink);
                                         auto read = [fd, &sink] (char* buf, int max_size) {
                                           ssize_t count = ::recv(fd, buf, max_size, 0);
                                           while (count <= 0)
                                           {
                                             if ((count < 0 and errno != EAGAIN) or count == 0)
                                               return ssize_t(0);
                                             sink = sink.resume();
                                             count = ::recv(fd, buf, max_size, 0);
                                           }
                                           return count;
                                         };

                                         auto write = [fd, &sink] (const char* buf, int size) {
                                           if (!buf or !size)
                                           {
                                             //std::cout << "pause" << std::endl;
                                             sink = sink.resume();
                                             return true;
                                           }
                                           const char* end = buf + size;
                                           ssize_t count = ::send(fd, buf, end - buf, MSG_NOSIGNAL);
                                           if (count > 0) buf += count;
                                           while (buf != end)
                                           {
                                             if ((count < 0 and errno != EAGAIN) or count == 0)
                                               return false;
                                             sink = sink.resume();
                                             count = ::send(fd, buf, end - buf, MSG_NOSIGNAL);
                                             if (count > 0) buf += count;
                                           }
                                           return true;
                                         };
              
                                         conn_handler(fd, read, write, listen_to_new_fd);
                                         close(fd);
                                         is_running[fd] = false;
                                         }
                                         catch (fiber_exception& ex) {
                                            //std::cerr << "my_exception: " << ex.what << std::endl;
                                            is_running[fd] = false;
                                            return std::move(ex.c);
                                         }
                                         //nrunning--;
                                         //std::cout << "nrunning: " << nrunning << std::endl;
                                         //epoll_ctl_del(fd);
                                         
                                         return std::move(sink);

                                       });
          
          }
          
        }
        else // Data available on existing sockets. Wake up the fiber associated with events[i].data.fd.
        {

          if (events[i].data.fd < secondary_map.size() && secondary_map[events[i].data.fd] != -1)
          {
            int original_fd = secondary_map[events[i].data.fd];
            if (fibers[original_fd])
              fibers[original_fd] = fibers[original_fd].resume();
          }
          else
            if (fibers[events[i].data.fd])
              fibers[events[i].data.fd] = fibers[events[i].data.fd].resume();
        }
      }
    }
    close(epoll_fd);
    return true;  
  };

  struct sigaction act;
    memset (&act, 0, sizeof(act));
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
