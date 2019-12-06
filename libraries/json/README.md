li::json
============================

li::json is a C++17 JSON serializer/deserializer designed for
ease of use and performances.

It handle a subset of other JSON serialization libraries: **Only cases
where the structure of the object is known at compile time are covered**.
It these specific cases, metajson is faster and produce smaller binaries.

**Features:**
  - Non intrusive
  - Header only
  - UTF-8 support
  - Exception free
  - Small codebase: 1200 LOC
  - Portable: No architecture specific code.

**Limitations:**
  - li::json only handles JSON objects with a **static structure known at compile time**.
  - li::json properly handle decoding and encoding UTF-8 but not the others UTF-{32|16} {big|little} endian encodings.
  - No explicit errors for ill-formatted json messsages.

**Performances:** Up to **9x** faster than nlohmann/json and **2x**
  faster than rapidjson*. I did not find usecases where li::json was
  not the fastest. If you find some, please report.

**Binary code size:** Up to **8x** smaller than nlohmann/json and **2x** smaller than rapidjson*.

[1] https://github.com/miloyip/rapidjson

[2] https://github.com/nlohmann/json

[3] https://github.com/AlDanial/cloc

\* Theses numbers are not given by an comprehensive benchmark. They just give a rough idea
of metajson performances and does not take into account the fact that other libraries provides
more features.

# Overview

Note: This example use the single header version.

```c++
#include "li_json.hh"

LI_SYMBOL(name)
LI_SYMBOL(age)
LI_SYMBOL(entry)
LI_SYMBOL(id)

int main ()
{
  using li_json::json_encode;
  using li_json::json_decode;
  using li_json::json_object;
  using li_json::json_vector;
  using li_json::json_key;
  using li_json::mmm;

  std::string json_str;

  // C-structs
  struct A { int age; std::string name; };
  A obj{12, "John"};

  json_str = json_object(s::age, s::name).encode(obj);
  json_object(s::age, s::name).decode(json_str, obj);

  std::cout << json_str << std::endl;
  // {"age":12,"name":"John"}

  // C++ vectors
  std::vector<int> v = {1,2,3,4};
  json_str = json_encode(v);
  json_decode(json_str, v);

  std::cout << json_str << std::endl;
  // [1,2,3,4]

  // Serialize getters as well as members.
  struct { int age; std::string name() { return "Bob"; } } gm;
  json_str = json_object(s::age, s::name).encode(gm);
  // {"age":12,"name":"Bob"}
  
  // std::tuple
  json_str = json_encode(std::make_tuple(1, "Bob", 3.4));
  std::cout << json_str << std::endl;
  // [1, "Bob", 3.4]

  // std::optional
  std::optional<std::string> my_optional_str;
  json_encode(my_optional_str); // empty string.
  my_optional_str = "lol";
  json_encode(my_optional_str); // "lol"

  // std::variant
  li::json_encode(std::variant<int,std::string>{"abc"});
  // {"idx":1,"value":"abc"}
  
  // Arrays of structs
  std::vector<A> array{ {12, "John"},  {2, "Alice"},  {32, "Bob"} };
  json_str = json_vector(s::age, s::name).encode(array);
  
  std::cout << json_str << std::endl;
  // [{"age":12,"name":"John"},{"age":2,"name":"Alice"},{"age":32,"name":"Bob"}]
 
  // Nested structs
  struct B { int id; A entry; };
  B obj2{ 1, { 12, "John"}};
  json_str = json_object(s::id = int(), s::entry = json_object(s::age, s::name)).encode(obj2);

  std::cout << json_str << std::endl;
  // {"id":1,"entry":{"age":12,"name":"John"}}

  // Metamap
  auto map = mmm(s::age = 12, s::name = std::string("John"));
  json_str = json_encode(map);
  
  std::cout << json_str << std::endl;
  // {"age":12,"name":"John"}
  
  json_decode(json_str, map);


  std::cout << json_object(s::age, s::name(json_key("last_name"))).encode(obj) << std::endl;
  // {"age":12,"last_name":"John"}

}

```

# What is the s:: namespace ?

Everything explained here: https://github.com/matt-42/lithium/tree/master/libraries/symbol#lisymbol

# Installation / Supported compilers

Everything explained here: https://github.com/matt-42/lithium#installation

# Authors

Matthieu Garrigues https://github.com/matt-42

# Support the project

If you find this project helpful, please consider donating:
https://www.paypal.me/matthieugarrigues
