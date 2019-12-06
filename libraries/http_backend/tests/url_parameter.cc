#include "test.hh"
#include <li/http_backend/request.hh>
#include <li/metamap/metamap.hh>

#include "symbols.hh"

using namespace li;

template <typename... O> auto url_decode_test(const char* spec, const char* url, O... param) {

  auto info = make_url_parser_info(spec);
  auto obj = mmm(param...);
  auto p = parse_url_parameters(info, url, obj);
  return p;
}

int main() {

  // std::cout << url_decode_test("/{{id}}", "/42", s::id = int()).id << std::endl;
  CHECK_EQUAL("1param", (url_decode_test("/{{id}}", "/42", s::id = int()).id), 42);
  CHECK_THROW("missing", (url_decode_test("/", "/42", s::id = int())));
  CHECK_THROW("spellerror", (url_decode_test("/{{idd}}", "/42", s::id = int())));
  CHECK_EQUAL(
      "param2",
      (url_decode_test("/x/sss/xxx/xx/{{id}}/xxx/ss/s", "/x/sss/xxx/xx/42/xxx/ss/s", s::id = int())
           .id),
      42);
  CHECK_EQUAL(
      "path",
      (url_decode_test("/x/{{path...}}", "/x/this/is/a/path", s::path = std::string()).path),
      "/this/is/a/path");

  auto p =
      url_decode_test("/hw/{{id}}/xxx/{{id2}}", "/hw/42/xxx/43", s::id = int(), s::id2 = int());
  assert(p.id == 42);
  assert(p.id2 == 43);
}