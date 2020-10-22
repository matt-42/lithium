#include <lithium_json.hh>
int main ()
{
  using namespace li;
  std::string json_str;
// __documentation_starts_here__
/*
json
============================

## Header

```c++
#include <lithium_json.hh>
```

## Introduction

``li::json`` is a C++17 JSON serializer/deserializer designed for
ease of use and performances. It can decode and encode Lithium metamaps (thanks to metamap
static introspection), standard C++ containers and any custom types (with some user provided hints)

It handle a subset of other JSON serialization libraries: **Only cases
where the structure of the object is known at compile time are covered**.
It these specific cases, metajson is faster and produce smaller binaries.

**Features:**
  - Non intrusive
  - Header only
  - UTF-8 support
  - Exception free
  - Small codebase: 1000 LOC
  - Portable: No architecture specific code.

**Limitations:**
  - It only handles JSON objects with a **static structure known at compile time**.
  - It properly handle decoding and encoding UTF-8 but not the others UTF-{32|16} {big|little} endian encodings.
  - No explicit errors for ill-formatted json messsages yet.

**Performances:** Up to **9x** faster than nlohmann/json [2] and **2x**
  faster than rapidjson [1]. I did not find usecases where it was
  not the fastest. If you find some, please report an issue.

**Binary code size:** Up to **8x** smaller than nlohmann/json and **2x** smaller than rapidjson*.

- [1] https://github.com/miloyip/rapidjson
- [2] https://github.com/nlohmann/json

Theses numbers are not given by an comprehensive benchmark. They just give a rough idea
of metajson performances and does not take into account the fact that other libraries provides
more features.

## Encoding

```c++
void json_encode(const O& object);
```

`json_encode` works out of the box for a set of types:
- standard C++ scalars: strings, integers, bool: encoded as json values
- lithium metamaps: encoded as json object
- `std::vector`: encoded as a json array
- `std::tuple`: encoded as a json array
- `std::variant`: encoded as {"idx": variant_index,"value": serialized_value}
- `std::optional`: when the optional is null, does not encode anything, if an object member is std::optional, skip it if is null.  
- `std::map`: encoded as json object
- `std::unordered_map`: encoded as json object

### Pointers deferencing

When `json_decode` meet an object pointer, it deferences it and serialize the pointer object.

### Encoding custom object types

Since there is no way for Lithium to know the members names your object, you need to describe the member or
accessor names whenever you serialize:
- a custom object
- a vector of custom objects 
- a map of custom objects

#### json_object

`json_object` takes the list of member or accessor names that you want to serialize.

*/
struct { 
  int age; 
  std::string name() { return "Bob"; } 
} gm;

json_str = json_object(s::age, s::name).encode(gm);

/*

#### `json_vector` for accessed with `array[int_index]`

`json_vector` takes the list of member or accessor names that you want to serialize from the vector elements.

*/

struct A { int age; std::string name; };
std::vector<A> array{ {12, "John"},  {2, "Alice"},  {32, "Bob"} };
json_str = json_vector(s::age, s::name).encode(array);

/*
#### `json_map` for maps accessed with `map[string_key]`

*/
std::unordered_map<std::string, int> test;

/*
Some examples:

### Metamaps

*/
  auto map = mmm(s::age = 12, s::name = std::string("John"));
  json_str = json_encode(map);
/*
### Metamaps

*/
  auto map = mmm(s::age = 12, s::name = std::string("John"));
  json_str = json_encode(map);
/*

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
