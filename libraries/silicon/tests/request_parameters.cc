#include <iod/http_client/http_client.hh>
#include <iod/silicon/silicon.hh>
#include "test.hh"

using namespace iod;

int main() {

  api<http_request, http_response> my_api;

  my_api.get("/get") = [&](http_request& request, http_response& response) {
    response.write(json_encode(request.get_parameters(s::id = int())));
  };
  my_api.post("/post") = [&](http_request& request, http_response& response) {
    response.write(json_encode(request.post_parameters(s::id = int())));
  };
  my_api.get("/url/{{id}}") = [&](http_request& request, http_response& response) {
    response.write(json_encode(request.url_parameters(s::id = int())));
  };

  auto ctx = http_serve(my_api, 12347, s::non_blocking);
  auto ref = json_encode(mmm(s::id = 42));

  // valid get.
  auto r1 = http_get("http://localhost:12347/get", s::get_parameters = mmm(s::id = 42));
  CHECK_EQUAL("get status", r1.status, 200);
  CHECK_EQUAL("get body", r1.body, ref);

  // missing get param.
  auto r2 = http_get("http://localhost:12347/get");
  CHECK_EQUAL("get missing status", r2.status, 400);

  // valid post
  auto r3 = http_post("http://localhost:12347/post", s::post_parameters = mmm(s::id = 42));
  CHECK_EQUAL("post", r3.status, 200);
  CHECK_EQUAL("post", r3.body, ref);
  // badly typed post
  auto r32 = http_post("http://localhost:12347/post", s::post_parameters = mmm(s::id = "id"));
  CHECK_EQUAL("post bad type", r32.status, 400);
  // missing post
  auto r4 = http_post("http://localhost:12347/post");
  CHECK_EQUAL("post missing", r4.status, 400);

  CHECK_EQUAL("url", http_get("http://localhost:12347/url/42").body, ref);
  CHECK_EQUAL("url invalid type", http_get("http://localhost:12347/url/xxx").status, 400);
}
