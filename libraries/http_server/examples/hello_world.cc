#include <lithium_http_server.hh>
            
int main() {
  li::http_api my_api;
              
  my_api.get("/hello_world") = 
  [&](li::http_request& request, li::http_response& response) {
    response.write("hello world.");
  };
  li::http_serve(my_api, 8080);
}
