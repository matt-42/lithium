#include <lithium_http_server.hh>
#include <lithium_http_client.hh>
#include "symbols.hh"

using namespace li;

int main() {

  http_api my_api;
  my_api.get("/") = [&](http_request& request, http_response& response) {
    response.write("hello world.");
  };

  http_serve(my_api, 12334, s::non_blocking);
  assert(http_get("http://localhost:12334/").body == "hello world.");

  http_api my_api2;
  my_api2.get("") = [&](http_request& request, http_response& response) {
    response.write("hello world.");
  };

  http_serve(my_api2, 12335, s::non_blocking);
  assert(http_get("http://localhost:12335/").body == "hello world.");
}
