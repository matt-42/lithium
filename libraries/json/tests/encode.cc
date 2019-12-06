#include <cassert>
#include <string_view>
#include <li/json/json.hh>
#include "symbols.hh"

using namespace li;

int main()
{

  {
    std::string input = R"json({"test1":12,"test2":"John"})json";

    auto obj = mmm(s::test1 = 12,
                                 s::test2 = std::string_view("John"));
  
    auto enc = json_encode(obj);
    assert(enc == input);

    std::ostringstream ss;
    json_encode(ss, obj);
    assert(ss.str() == input);
  
    struct { int test1() { return 12; }; std::string test2; } obj2{"John"};  
    assert(json_object(s::test1, s::test2).encode(obj2) == input);

    ss.str("");
    json_object(s::test1, s::test2).encode(ss, obj2);
    assert(ss.str() == input);
  }

  
  
  {
    // Arrays;
    std::string input = R"json({"test1":2,"test2":[{"test1":11,"test2":"Bob"},{"test1":12,"test2":"John"}]})json";
  
    struct A { int test1; std::string test2; };
    struct { int test1; std::vector<A> test2; } obj{2, { {11, "Bob"}, {12, "John"} }};

    assert(json_object(s::test1, s::test2 = json_vector(s::test1, s::test2)).encode(obj) == input);
  }

  {
    // json_key
    std::string input = R"json({"test1":12,"name":"John"})json";

    auto obj = li::mmm(s::test1 = 12,
                                 s::test2 = std::string_view("John"));
  
    assert(input == li::json_object(s::test1, s::test2(li::json_key("name"))).encode(obj));
  }

  {
    // Nested array
    std::string input = R"json([{"test1":["test1":12]}])json";

    using s::test1;
    typedef decltype(li::mmm(s::test1 = std::vector<decltype(li::mmm(s::test1 = int()))>())) elt;
    auto obj = std::vector<elt>();
    obj.push_back(li::mmm(s::test1 = { li::mmm(s::test1 = 12) }));
  }

  {
    // plain vectors.
    std::string input = R"json([1,2,3,4])json";
    assert(json_encode(std::vector<int>{1,2,3,4}) == input);
  }
  
  {
    // tuples.
    std::string input = R"json([42,"foo",0])json";
    assert(json_encode(std::make_tuple(42,"foo",0)) == input);
  }

  {
    // nested tuples.
    std::string input = R"json([42,"foo",0,[32,"Bob"]])json";
    assert(json_encode(std::make_tuple(42,"foo",0, std::make_tuple(32,"Bob"))) == input);
  }

  {
    // tuples with struct element.
    std::string input = R"json(["Alice",{"test1":11,"test2":"Bob"}])json";

    struct A { int test1; std::string test2; };
    auto A_json = json_object(s::test1, s::test2);
    auto tu = std::make_tuple("Alice", A{11, "Bob"});
    assert(json_tuple(std::string(), A_json).encode(tu) == input);
  }

  {
    // Optionals.
    struct { std::optional<std::string> test2; } x;
    assert(json_object(s::test2).encode(x) == "{}");

    x.test2 = "he";
    assert(json_object(s::test2).encode(x) == R"json({"test2":"he"})json");
    
  }

  {
    // Simple values.
    assert(json_encode(12) == "12");
    assert(json_encode("12") == "\"12\"");
    assert(json_encode(std::optional<int>{}) == "");
    assert(json_encode(std::optional<int>{12}) == "12");
    assert(json_encode(std::variant<int,std::string>{"abc"}) == R"json({"idx":1,"value":"abc"})json");
    assert(json_encode(std::variant<int,std::string>{42}) == R"json({"idx":0,"value":42})json");
  }
}
