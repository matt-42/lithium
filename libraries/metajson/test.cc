#include "metajson.hh"
#include <iostream>

IOD_SYMBOL(name)
IOD_SYMBOL(age)
IOD_SYMBOL(entry)
IOD_SYMBOL(id)

int main ()
{
  using iod::json_encode;
  using iod::json_decode;

  using iod::json_object;
  using iod::json_vector;
  
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
  using iod::make_metamap;
  auto map = make_metamap(s::age = 12, s::name = std::string("John"));
  json_str = json_encode(map);
  
  std::cout << json_str << std::endl;
  // {"age":12,"name":"John"}
  
  json_decode(json_str, map);


  std::cout << json_object(s::age, s::name(iod::json_key("last_name"))).encode(obj) << std::endl;
  // {"age":12,"last_name":"John"}

}
