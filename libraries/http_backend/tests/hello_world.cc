#include <li/http_backend/http_backend.hh>
#include <li/http_client/http_client.hh>

using namespace li;

int main() {

  http_api my_api;

  my_api.get("/hello_world") = [&](http_request& request, http_response& response) {
    response.write("hello ", request.get_parameters(s::name = std::string()).name, ".");
  };
  my_api.get("/hello_world2") = [&](http_request& request, http_response& response) {
    response.write("hello world.");
  };

  auto ctx = http_serve(my_api, 12344, s::non_blocking);

  assert(http_get("http://localhost:12344/hello_world", s::get_parameters = mmm(s::name = "John"))
             .body == "hello John.");
}
