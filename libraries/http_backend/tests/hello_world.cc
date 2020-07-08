#include <lithium.hh>


using namespace li;

int main() {

  http_api my_api;

  my_api.get("/hello_world") = [&](http_request& request, http_response& response) {
    response.write("hello world.");
  };

  //http_serve(my_api, 12334);
  http_serve(my_api, 12334, s::non_blocking);
  assert(http_get("http://localhost:12334/hello_world").body == "hello world.");
}
