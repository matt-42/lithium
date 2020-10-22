#include <lithium_http_server.hh>
#include <lithium_http_client.hh>
#include "symbols.hh"

using namespace li;

int main() {

  http_api my_api;

  my_api.get("/hello_world") = [&](http_request& request, http_response& response) {
    response.write("hello world.");
  };

  //http_serve(my_api, 12335, s::nthreads = 1);
  http_serve(my_api, 12334, s::non_blocking);
  assert(http_get("http://localhost:12334/hello_world").body == "hello world.");
}
