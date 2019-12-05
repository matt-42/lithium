#include "test.hh"
#include <li/http_backend/http_backend.hh>
#include <li/http_client/http_client.hh>

using namespace li;

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
  my_api.post("/optional_not_set") = [&](http_request& request, http_response& response) {
    assert(!request.get_parameters(s::id = std::optional<int>()).id);
    assert(!request.post_parameters(s::id = std::optional<int>()).id);
  };
  my_api.post("/optional_set") = [&](http_request& request, http_response& response) {
    assert(request.get_parameters(s::id = std::optional<int>()).id.value() == 42);
    assert(request.post_parameters(s::id = std::optional<int>()).id.value() == 43);
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

  // valid post JSON
  auto r31 = http_post("http://localhost:12347/post", s::post_parameters = mmm(s::id = 42),
                       s::jsonencoded);
  CHECK_EQUAL("post", r31.status, 200);
  CHECK_EQUAL("post", r31.body, ref);

  // badly typed post
  auto r32 = http_post("http://localhost:12347/post", s::post_parameters = mmm(s::id = "id"));
  CHECK_EQUAL("post bad type", r32.status, 400);
  // missing post
  auto r4 = http_post("http://localhost:12347/post");
  CHECK_EQUAL("post missing", r4.status, 400);

  CHECK_EQUAL("url", http_get("http://localhost:12347/url/42").body, ref);
  CHECK_EQUAL("url invalid type", http_get("http://localhost:12347/url/xxx").status, 400);

  CHECK_EQUAL("not setting optionals.", http_post("http://localhost:12347/optional_not_set").status,
              200);
  auto r5 = http_post("http://localhost:12347/optional_set", s::get_parameters = mmm(s::id = 42),
                      s::post_parameters = mmm(s::id = 43));
  std::cout << json_encode(r5) << std::endl;
  CHECK_EQUAL("setting optionals.", r5.status, 200);
}
