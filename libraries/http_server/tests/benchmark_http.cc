#include <string>
#include <cstring>

#include <li/http_server/http_server.hh>
#include <li/http_server/http_benchmark.hh>


//  other file descriptors events.
struct benchmark_fiber {

  typedef std::runtime_error exception_type;

  inline benchmark_fiber& operator=(const benchmark_fiber&) = delete;
  inline benchmark_fiber(const benchmark_fiber&) = delete;

  inline void yield() { }

  inline ~benchmark_fiber() {}

  inline void epoll_add(int fd, int flags) {}
  inline void epoll_mod(int fd, int flags) {}
  inline void reassign_fd_to_this_fiber(int fd) {}

  inline void defer(const std::function<void()>& fun) {}
  inline void defer_fiber_resume(int fiber_id) {}

  inline int read(char* buf, int max_size) {

    if (read_buffer_pos == read_buffer.size() && nreqs < max_requests)
    {
      nreqs++;
      read_buffer_pos = 0;
    }

    int end = std::min(read_buffer_pos + max_size, int(read_buffer.size()));
    int count = end - read_buffer_pos;
    memcpy(buf, read_buffer.data() + read_buffer_pos, count);
    read_buffer_pos += count;
    return count;
  };

  inline bool write(const char* buf, int size) {
    return true;
  };

  int max_requests;
  std::string read_buffer;
  int read_buffer_pos = 0;
  int nreqs = 0;  
  int socket_fd;
};

int main() {


  auto handler = [] (auto& ctx) {};
  auto http = li::http_async_impl::make_http_processor(handler);

  int N = 3500000;
  benchmark_fiber input{N, "GET /cookies HTTP/1.1\r\nHost: 127.0.0.1:8090\r\nConnection: keep-alive\r\nCache-Control: max-age=0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nUser-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.17 (KHTML, like Gecko) Chrome/24.0.1312.56 Safari/537.17\r\nAccept-Encoding: gzip,deflate,sdch\r\nAccept-Language: en-US,en;q=0.8\r\nAccept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.3\r\nCookie: name=wookie\r\n\r\n"};

  li::http_benchmark_impl::timer t;
  t.start();
  http(input);
  t.end();

  std::cout << (1000. * N / t.ms())  << " req/s" << std::endl;


  return 0;
}
