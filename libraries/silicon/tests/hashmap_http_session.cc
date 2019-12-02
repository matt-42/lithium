#include <iod/http_client/http_client.hh>
#include <iod/silicon/silicon.hh>
#include <iod/silicon/sql_http_session.hh>

#include "symbols.hh"
#include "test.hh"

using namespace iod;

int main() {

  api<http_request, http_response> my_api;

  // Session.
  hashmap_http_session session(s::user_id = -1);

  my_api.get("/test1") = [&](http_request& request, http_response& response) {
    auto sess = session.connect(request, response);
    CHECK_EQUAL("default", sess.values().user_id, -1);
    sess.values().user_id = 42;
  };

  my_api.get("/test2") = [&](http_request& request, http_response& response) {
    auto sess = session.connect(request, response);
    CHECK_EQUAL("id", sess.values().user_id, 42);
  };

  my_api.get("/test3") = [&](http_request& request, http_response& response) {
    auto sess = session.connect(request, response);
    sess.store(s::user_id = 42);
  };

  my_api.get("/test4") = [&](http_request& request, http_response& response) {
    auto sess = session.connect(request, response);
    CHECK_EQUAL("id", sess.values().user_id, 42);
  };

  my_api.get("/test5") = [&](http_request& request, http_response& response) {
    auto sess = session.connect(request, response);
    sess.logout();
  };
  my_api.get("/test6") = [&](http_request& request, http_response& response) {
    auto sess = session.connect(request, response);
    CHECK_EQUAL("id", sess.values().user_id, -1);
  };

 
  auto ctx = http_serve(my_api, 12351, s::non_blocking);
  auto c = http_client("http://localhost:12351");

  c.get("/test1");
  c.get("/test2");
  c.get("/test3");
  c.get("/test4");
  c.get("/test5");
  c.get("/test6");
}
