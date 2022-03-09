#include "symbols.hh"
#include <cassert>
#include <lithium_json.hh>

using namespace li;

int main() {
  { // Simple deserializer.
    std::string input = R"json({"test1":12,"test2":"John"})json";

    auto obj = mmm(s::test1 = int(), s::test2 = std::string());

    json_decode(input, obj);
    assert(obj.test1 == 12);
    assert(obj.test2 == "John");
  }

  { // C-struct with getter and members
    std::string input = R"json({"test1":12,"test2":42})json";

    struct {
      int& test1() { return tmp; }
      int test2;

    private:
      int tmp;
    } a;

    auto err = json_object(s::test1, s::test2).decode(input, a);
    assert(!err);
    assert(a.test1() == 12);
    assert(a.test2 == 42);
  }

  { // json key.
    std::string input = R"json({"test1":12,"name":"John"})json";

    auto obj = mmm(s::test1 = int(), s::test2 = std::string());

    auto err = json_object(s::test1, s::test2(json_key("name"))).decode(input, obj);
    if (err)
      std::cout << err.what << std::endl;
    assert(obj.test1 == 12);
    assert(obj.test2 == "John");
  }

  {
    // plain vectors.
    std::string input = R"json([1,2,3,4])json";

    std::vector<int> v;
    auto err = json_decode(input, v);
    assert(!err);
    assert(v.size() == 4);
    assert(v[0] == 1);
    assert(v[1] == 2);
    assert(v[2] == 3);
    assert(v[3] == 4);
  }

  {
    // plain vectors.
    std::string input = R"json([{"test1":12}])json";

    struct A {
      int test1;
    };

    std::vector<A> v;
    auto err = json_vector(s::test1).decode(input, v);
    assert(!err);
    assert(v.size() == 1);
    assert(v[0].test1 == 12);
  }

  {
    // static vector.
    int arr[2];
    json_decode(R"json( [  23 , 45 ] )json", arr);
    assert(arr[0] == 23);
    assert(arr[1] == 45);
  }

  {
    // tuples.
    std::string input = R"json( [  42  ,  "foo" , 0 , 4 ] )json";

    std::tuple<int, std::string, bool, int> tu;
    auto err = json_decode(input, tu);
    assert(!err);
    assert(std::get<0>(tu) == 42);
    assert(std::get<2>(tu) == 0);
    assert(std::get<3>(tu) == 4);
  }

  {
    // optional.
    auto obj = mmm(s::test1 = std::optional<std::string>());
    auto err = json_decode("{}", obj);
    assert(err.good());
    assert(!obj.test1.has_value());
    err = li::json_decode("{\"test1\": \"Hooh\"}", obj);
    assert(err.good());
    assert(obj.test1.has_value());
    assert(obj.test1.value() == "Hooh");
  }

  {
    // Variant.
    auto obj = mmm(s::test1 = std::variant<int, std::string>("abc"));

    assert(json_decode(R"json({"test1":{"idx":1,"value":"abcd"}})json", obj).good());
    assert(std::get<std::string>(obj.test1) == "abcd");

    assert(json_decode(R"json({"test1":{"idx":0,"value":42}})json", obj).good());
    assert(std::get<int>(obj.test1) == 42);

    // Variant of custom objects.
    struct X {
      int test2;
    };
    struct Y {
      int test1;
    };

    auto json_info = json_variant(json_object(s::test2), json_object(s::test1));

    auto v = std::variant<X, Y>(X{23});
    assert(v.index() == 0);
    assert(std::get<X>(v).test2 == 23);
    assert(json_info.decode(R"json({"idx":0,"value":{"test2":42}})json", v).good());
    assert(std::get<X>(v).test2 == 42);

    assert(json_info.decode(R"json({"idx":1,"value":{"test1":51}})json", v).good());
    assert(std::get<Y>(v).test1 == 51);
  }

  {
    // Map.
    auto obj = std::unordered_map<std::string, int>();

    assert(json_decode(R"json({"test1":1, "test2": 4})json", obj).good());
    assert(obj["test1"] == 1);
    assert(obj["test2"] == 4);
  }
}
