#include <iod/http_client/http_client.hh>
#include <iod/silicon/silicon.hh>

using namespace iod;

int main() {

  api<http_request, http_response> my_api;

  my_api(GET, "/hello_world") = [&](http_request& request, http_response& response) {
    response.write("hello world.");
  };
  my_api(GET, "/hello_world2") = [&](http_request& request, http_response& response) {
    response.write("hello world.");
  };

  auto ctx = http_serve(my_api, 12345, s::non_blocking);

  assert(http_get("http://localhost:12345/hello_world").body == "hello world.");
}
