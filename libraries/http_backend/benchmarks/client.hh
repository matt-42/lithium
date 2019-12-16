#pragma once

#include <atomic>
#include <iostream>
#include <thread>
#include <chrono>
#include <boost/context/continuation.hpp>

namespace ctx = boost::context;

class timer
{
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

void error(const char *msg) {
    perror(msg);
    exit(0);
}


float benchmark(int NCONNECTIONS, int NTHREADS, int duration_in_seconds,
               int port, std::string_view req)
{
  auto client_fn = [port, NCONNECTIONS] (auto conn_handler) {

    int sockets[NCONNECTIONS];
    int portno, n;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    const char *hostname;

    /* check command line arguments */
    hostname = "0.0.0.0";
    portno = port;

    /* socket: create the socket */
    for (int i = 0; i < NCONNECTIONS; i++)
    {
      sockets[i] = socket(AF_INET, SOCK_STREAM, 0);
      if (sockets[i] < 0) 
        error("ERROR opening socket");
    }

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
      fprintf(stderr,"ERROR, no such host as %s\n", hostname);
      exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
          (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    /* connect: create a connection with the server */    
    for (int i = 0; i < NCONNECTIONS; i++)
      if (connect(sockets[i], (const sockaddr*) &serveraddr, sizeof(serveraddr)) < 0) 
        error("ERROR connecting");

    for (int i = 0; i < NCONNECTIONS; i++)
      fcntl(sockets[i], F_SETFL, fcntl (sockets[i], F_GETFL, 0) | O_NONBLOCK);

    int epoll_fd = epoll_create1(0);

    auto epoll_ctl = [epoll_fd] (int fd, int op, uint32_t flags)
    {
      epoll_event event;
      event.data.fd = fd;
      event.events = flags;
      MOUSTIQUE_CHECK_CALL(::epoll_ctl(epoll_fd, op, fd, &event));
      return true;
    };

    for (int i = 0; i < NCONNECTIONS; i++)
      epoll_ctl(sockets[i], EPOLL_CTL_ADD, EPOLLIN | EPOLLOUT | EPOLLET);

    const int MAXEVENTS = 64;
    std::vector<ctx::continuation> fibers;
    int ncompleted = 0;
    int nstarted = 0;
    for (int i = 0; i < NCONNECTIONS; i++)
    {
      int infd = sockets[i];
      if (int(fibers.size()) < infd + 1)
        fibers.resize(infd + 10);
      
      fibers[infd] = ctx::callcc([fd=infd, &conn_handler, epoll_ctl,&ncompleted, &nstarted]
                                 (ctx::continuation&& sink) {
                                   auto read = [fd, &sink, epoll_ctl] (char* buf, int max_size) {
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

                                   auto write = [fd, &sink, epoll_ctl] (const char* buf, int size) {
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
                                  nstarted++;
                                   std::cout << "enter callcc " << nstarted << std::endl;
                                   conn_handler(fd, read, write);
                                   ncompleted++;
                                   std::cout << "return callcc " << ncompleted << std::endl;
                                   close(fd);
                                   return std::move(sink);
                                 });
    }
      
    // Even loop.
    epoll_event events[MAXEVENTS];
    while (true)
    {
      int n_events = epoll_wait (epoll_fd, events, MAXEVENTS, 100);
      for (int i = 0; i < n_events; i++)
      {
        if ((events[i].events & EPOLLERR) ||
            (events[i].events & EPOLLHUP))
        {
          close (events[i].data.fd);
          fibers[events[i].data.fd] = fibers[events[i].data.fd].resume();
          continue;
        }
        else // Data available on existing sockets. Wake up the fiber associated with events[i].data.fd.
          fibers[events[i].data.fd] = fibers[events[i].data.fd].resume();
      }

      if (ncompleted == NCONNECTIONS)
        break;
    }
  };

  int nconn = 100;
  std::atomic<int> nmessages = 0;

  timer global_timer;
  global_timer.start();

  bool ended = false;
  auto bench_tcp = [&] () {

    client_fn([&] (int fd, auto read, auto write)
    { // flood the server.
      timer t; t.start();
      while(true)
      {
        char buf_read[10000];
        int tosend = 1e4/NCONNECTIONS;
        for (int i = 0; i < tosend; i++)
        {
          write(req.data(), req.size());
          write(req.data(), req.size());
          // write(req.data(), req.size());
          int rd = read(buf_read, sizeof(buf_read));
          // rd = read(buf_read, sizeof(buf_read));
          //rd = read(buf_read, sizeof(buf_read));
          printf("%.*s\n", rd, buf_read);
          rd = read(buf_read, sizeof(buf_read));
          printf("%.*s\n", rd, buf_read);
          return;
          break;
        }
        t.end();
        nmessages += tosend;
        //std::cout << t.ms() << std::endl; 
        if (ended)
        {
          std::cout << "end connection" << std::endl;
          break;
        }
      }

    });
    
  };
  std::thread monitoring(
    [&] {
      timer t; t.start();
      while (true)
      {
        usleep(2e6);
        if (ended) break;
        t.end();       
        if (t.ms() > 1000*duration_in_seconds) ended = true;
        std::cout << (1000.*nmessages /t.ms()) << " req/s" << std::endl;
      }
    });

  int nthreads = NTHREADS;
  //bench_tcp();
  std::vector<std::thread> ths;
  for (int i = 0; i < nthreads; i++)
    ths.push_back(std::thread(bench_tcp));
  for (auto& t : ths)
    t.join();
  ended = true;
  global_timer.end();
  std::cout << nmessages << " messages in " << global_timer.ms() << " ms." << std::endl;
  std::cout << (1000.*nmessages /global_timer.ms()) << " req/s" << std::endl;

  monitoring.join();
  return (1000.*nmessages /global_timer.ms());
}
