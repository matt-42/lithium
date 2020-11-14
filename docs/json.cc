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
ease of use and performances. It's main advantage over other libraries is that it
feeds directly plain C++ object with the json string. No intermediate data structure 
is allocated. Other JSON libraries usually encode/decode JSON from/to dynamic data
structure like dictionary or hash map to store data.


It can decode and encode standard C++ containers, Lithium metamaps, and any custom 
types (you need to provide some type info for this, more on this bellow).

The drawback of this "no intermediate dynamic data structure" optimization, is that it handles a
subset of other JSON serialization libraries: **Only cases
where the structure of the object is known at compile time are covered**.
In these specific cases, metajson is faster and produce smaller binaries.

### Features
  - Non intrusive
  - Header only
  - UTF-8 support
  - Exception free
  - Small codebase: 1000 LOC
  - Portable: No architecture specific code.

### Limitation
  - It only handles JSON objects with a **static structure known at compile time**.
  - It properly handle decoding and encoding UTF-8 but not the others UTF-{32|16} {big|little} endian encodings.
  - No explicit errors for ill-formatted json messsages yet.

### Performances

**Runtime:** Up to **9x** faster than nlohmann/json [2] and **2x**
faster than rapidjson [1]. I did not find usecases where it was
not the fastest. If you find some, please report an issue.

**Binary code size:** Up to **8x** smaller than nlohmann/json and **2x** smaller than rapidjson*.

- [1] https://github.com/miloyip/rapidjson
- [2] https://github.com/nlohmann/json

Theses numbers are not given by an comprehensive benchmark. They just give a rough idea
of metajson performances and does not take into account the fact that other libraries provides
more features.

### Dependencies

None

## Encoding

There is two ways to decode json objects: one for [supported types](#json/supported-types) and one for [custom 
types](#json/providing-json-type-info-for-custom-types) where you have to provide some information about the 
object you want to encode:
```c++
// Supported types
std::string json_encode(input_object);
void json_encode(output_stream, input_object);

// Custom types
std::string __your_type_info__.encode(input_object);
void __your_type_info__.encode(output_stream, input_object);
```

When `output_stream` is passed as first paramenter, the json encode will use output_stream.operator<< to 
encode the json object otherwise an instance of `std::ostringstream` is used.

When elements of a json array can be computed on the fly, you can save a vector allocation by using the
`json_encode_generator` and `_json_vector_type_info_.encode_generator` function:

*/
// Encode json array from element without allocating a vector.
int i = 0;
auto g = [&i] { return i++; };
assert(json_encode_generator(3, g) == "[0,1,2]");
/*

## Decoding

```c++
// Supported types
void json_decode(json_string, output_object); 

// Custom types
void __your_type_info__.decode(json_string, output_object);
```

## Supported types

`json_encode` and `json_decode` works out of the box for a set of types:
- standard C++ scalars: strings, integers, bool encoded as json values
- lithium metamaps: encoded as json object
- `std::vector`: encoded as a json array
- `std::tuple`: encoded as a json array
- `std::variant`: encoded as {"idx": variant_index,"value": serialized_value}
- `std::optional`: when the optional is null, does not encode anything, if an object member is std::optional, skip it if is null.  
- `std::map`: encoded as json object
- `std::unordered_map`: encoded as json object

## Providing json type info for custom types

Since there is no way for Lithium to know the members names your objects, you need to provide the list of member or
accessor names (type infos) that you want to encode or decode.
The following sections describe the functions that you can use to build type infos:

#### json_object

`json_object` takes the list of member or accessor names that you want to serialize.

*/
struct { 
  int age; 
  std::string name() { return "Bob"; } 
} gm{42};

json_str = json_object(s::age, s::name).encode(gm);
// {"age":42,"name":"Bob"}

/*

#### json_vector

Use `json_vector` to encode vectors storing a custom object type. 
Like `json_object` it takes the member of accessor names to serialize:

*/

struct A { int age; std::string name; };
std::vector<A> array{ {12, "John"},  {2, "Alice"},  {32, "Bob"} };
json_str = json_vector(s::age, s::name).encode(array);
// [{"age":12,"name":"John"},{"age":2,"name":"Alice"},{"age":32,"name":"Bob"}]

/*
#### json_map

Behave like `json_vector` but for maps. Note that only maps with string as key type are serializable.

*/

std::unordered_map<std::string, A> test{ 
   {"a", {12, "John"}}, 
   {"b",  {2, "Alice"}}};

json_str = json_map(s::age, s::name).encode(array);
//  {"a" : {"age":12,"name":"John"}, "b" : {"age":2,"name":"Alice"}}

/*

#### json_tuple

Take as argument the types of the tuple elements. If one of those types contain custom
types, use `json_[object|vector|map|tuple]`:

/*/

auto t = std::make_tuple("Alice", A{11, "Bob"});
json_str = json_tuple(std::string(), json_object(s::name, s::age)).encode(t);
// ["Alice",{"age":11,"name":"Bob"}]";

/*

#### Nested custom types

If a member of an object is also a custom type, you can nest several definitions:
*/
json_vector(s::id, s::info = json_object(s::name, s::age));
/*
#### Custom JSON keys

By default, JSON keys are mapped to object member/accessor names. You can overide this behavior
by providing a custom json key:
*/
json_object(s::test1, s::test2(li::json_key("name")))
// member test2 will be encoded as "name".

/*


### Pointers deferencing

When `json_decode` meet an object pointer, it dereferences it and serialize the pointer object.

*/



/*
*/
}
