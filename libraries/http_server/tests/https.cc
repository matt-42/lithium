#include <lithium_http_server.hh>
#include <lithium_http_client.hh>
#include "symbols.hh"

using namespace li;

int main() {

  http_api my_api;

  my_api.get("/hello_world") = [&](http_request& request, http_response& response) {
    response.write("hello world.");
  };
  system("openssl req -new -newkey rsa:4096 -x509 -sha256 -days 365 -nodes -out ./server.crt -keyout ./server.key -subj \"/CN=localhost\"");
  http_serve(my_api, 12335, s::non_blocking, s::ssl_key = "./server.key", s::ssl_certificate = "./server.crt", s::ssl_ciphers = "ALL:!NULL");
  assert(http_get("https://localhost:12335/hello_world", s::disable_check_certificate).body == "hello world.");
}
