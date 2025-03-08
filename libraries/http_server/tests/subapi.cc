#include <lithium_http_server.hh>
#include <lithium_http_client.hh>
#include "symbols.hh"

using namespace li;

int main() {

  http_api root;
  root.get("/") = [&](http_request& request, http_response& response) { response.write("root"); };

  http_api subapi;
  subapi.get("/") = [&](http_request& request, http_response& response) {
    response.write("hello");
  };

  subapi.get("/world") = [&](http_request& request, http_response& response) {
    response.write("hello world");
  };

  root.add_subapi("/hello", subapi);

  http_serve(root, 12334, s::non_blocking);
  assert(http_get("http://localhost:12334").body == "root");
  assert(http_get("http://localhost:12334/").body == "root");
  assert(http_get("http://localhost:12334/hello").body == "hello");
  assert(http_get("http://localhost:12334/hello/").body == "hello");
  assert(http_get("http://localhost:12334/hello/world").body == "hello world");
}
