#include "symbols.hh"
#include <cassert>
#include <lithium_json.hh>
#include <string_view>

using namespace li;

int main() {
  
  {
    std::string input = R"json({"test1":12,"test2":"John"})json";

    auto obj = mmm(s::test1 = 12, s::test2 = std::string_view("John"));

    auto enc = json_encode(obj);
    assert(enc == input);

    std::ostringstream ss;
    json_encode(ss, obj);
    assert(ss.str() == input);

    struct {
      int test1() { return 12; };
      std::string test2;
    } obj2{"John"};
    assert(json_object(s::test1, s::test2).encode(obj2) == input);

    ss.str("");
    json_object(s::test1, s::test2).encode(ss, obj2);
    assert(ss.str() == input);
  }

  {
    // Arrays;
    std::string input =
        R"json({"test1":2,"test2":[{"test1":11,"test2":"Bob"},{"test1":12,"test2":"John"}],"test3":[0,1,2]})json";

    struct A {
      int test1;
      std::string test2;
    };
    struct {
      int test1;
      std::vector<A> test2;
      std::vector<int> test3;
    } obj{2, {{11, "Bob"}, {12, "John"}}, {0,1,2}};

    std::cout << json_object(s::test1, 
    s::test2 = json_object_vector(s::test1, s::test2),
    s::test3).encode(obj) << std::endl;
    assert(json_object(s::test1, 
    s::test2 = json_object_vector(s::test1, s::test2),
    s::test3).encode(obj) == input);

  }

  {
    // json_key
    std::string input = R"json({"test1":12,"name":"John"})json";

    auto obj = li::mmm(s::test1 = 12, s::test2 = std::string_view("John"));

    assert(input == li::json_object(s::test1, s::test2(li::json_key("name"))).encode(obj));
  }

  {
    // Nested array
    std::string input = R"json([{"test1":["test1":12]}])json";

    using s::test1;
    typedef decltype(li::mmm(s::test1 = std::vector<decltype(li::mmm(s::test1 = int()))>())) elt;
    auto obj = std::vector<elt>();
    obj.push_back(li::mmm(s::test1 = {li::mmm(s::test1 = 12)}));
  }

  {
    // plain vectors.
    std::string input = R"json([1,2,3,4])json";
    assert(json_encode(std::vector<int>{1, 2, 3, 4}) == input);
    assert(json_encode(std::vector<char>{1, 2, 3, 4}) == input);
    assert(json_encode(std::vector<uint8_t>{1, 2, 3, 4}) == input);
  }

  {
    // static vectors.
    int arr[4] = {1,2,3,4};
    std::cout << json_encode(arr) << std::endl;
    assert(json_encode(arr) ==  R"json([1,2,3,4])json");

    li::metamap<s::test1_t::variable_t<int>> arr2[2] = {li::mmm(s::test1 = 1), li::mmm(s::test1 = 2)};
    std::cout << json_encode(arr2) << std::endl; 
    assert(json_encode(arr2) ==  R"json([{"test1":1},{"test1":2}])json");
  
    struct X { float t1; }; 
    X arr3[] = { {1.2}, {2.1} };
    std::cout << json_static_array<2>(s::t1 = float()).encode(arr3) << std::endl; 
    assert(json_static_array<2>(s::t1).encode(arr3) ==  R"json([{"t1":1.2},{"t1":2.1}])json");

    struct X2 { float t1[2]; }; 
    X2 testX2{ {1.234, 2.3} };
    std::cout << json_object(s::t1).encode(testX2) << std::endl;
    assert(json_object(s::t1).encode(testX2) ==  R"json({"t1":[1.234,2.3]})json");

  }


  {
    // tuples.
    std::string input = R"json([42,"foo",0])json";
    assert(json_encode(std::make_tuple(42, "foo", 0)) == input);
  }

  {
    // nested tuples.
    std::string input = R"json([42,"foo",0,[32,"Bob"]])json";
    assert(json_encode(std::make_tuple(42, "foo", 0, std::make_tuple(32, "Bob"))) == input);
  }

  {
    // tuples with struct element.
    std::string input = R"json(["Alice",{"test1":11,"test2":"Bob"}])json";

    struct A {
      int test1;
      std::string test2;
    };
    auto A_json = json_object(s::test1, s::test2);
    auto tu = std::make_tuple("Alice", A{11, "Bob"});
    assert(json_tuple(std::string(), A_json).encode(tu) == input);
  }

  {
    // Optionals.
    struct {
      std::optional<std::string> test2;
    } x;
    assert(json_object(s::test2).encode(x) == "{}");

    x.test2 = "he";
    assert(json_object(s::test2).encode(x) == R"json({"test2":"he"})json");
  }

  {
    // Simple values.
    assert(json_encode(12) == "12");
    std::cout << json_encode("12") << std::endl;
    assert(json_encode("12") == "\"12\"");
    assert(json_encode(std::optional<int>{}) == "");
    assert(json_encode(std::optional<int>{12}) == "12");
    assert(json_encode(std::variant<int, std::string>{"abc"}) ==
           R"json({"idx":1,"value":"abc"})json");
    assert(json_encode(std::variant<int, std::string>{42}) == R"json({"idx":0,"value":42})json");
  }

  {
    // Variant of custom objects
    struct X {
      int test2;
    };
    struct Y {
      int test1;
    };

    auto json_info = json_variant(json_object(s::test2), json_object(s::test1));

    std::cout << "json_info.encode(std::variant<X, Y>(X{32}))" << std::endl;
    std::cout << json_info.encode(std::variant<X, Y>(X{32})) << std::endl;
    assert(json_info.encode(std::variant<X, Y>(X{32})) == R"json({"idx":0,"value":{"test2":32}})json");
    assert(json_info.encode(std::variant<X, Y>(Y{33})) == R"json({"idx":1,"value":{"test1":33}})json");
  }

  {
    // Maps.
    std::unordered_map<std::string, int> test;
    assert(json_encode(test) == "{}");

    test["test1"] = 2;
    test["test2"] = 4;

    std::cout << json_encode(test) << std::endl;

    std::string encoded = json_encode(test);
    std::unordered_map<std::string, int> test2;

    json_decode(encoded, test2);
    assert(test2["test1"] == 2);
    assert(test2["test2"] == 4);
  }

  {
    // Pointers.
    std::string input = R"json([1,2,3,4])json";
    auto to_encode = std::vector<int>{1, 2, 3, 4};
    std::cout << json_encode(&to_encode) << std::endl;
    assert(json_encode(&to_encode) == input);
  }

  {
    // Generators.
    int i = 0;
    auto g = [&i] { return i++; };
    assert(json_encode_generator(3, g) == "[0,1,2]");
  }
}
