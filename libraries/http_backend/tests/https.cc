#include <li/http_backend/http_backend.hh>
#include <li/http_client/http_client.hh>

using namespace li;

int main() {

  http_api my_api;

  my_api.get("/hello_world") = [&](http_request& request, http_response& response) {
    response.write("hello world.");
  };

  system("openssl req -new -newkey rsa:4096 -x509 -sha256 -days 365 -nodes -out ./server.crt -keyout ./server.key -subj \"/CN=localhost\"");
  http_serve(my_api, 12334, s::non_blocking, s::ssl_key = "./server.key", s::ssl_certificate = "./server.crt");
  assert(http_get("https://localhost:12334/hello_world", s::disable_check_certificate).body == "hello world.");
}
