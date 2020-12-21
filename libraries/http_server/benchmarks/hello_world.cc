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
  auto sockets = http_benchmark_connect(100, port);
  float req_per_s = http_benchmark(sockets, 1, 2000, "GET /hello_world HTTP/1.1\r\n\r\n");
  std::cout << req_per_s << " req/s." << std::endl;
  
  http_benchmark_close(sockets);
}
