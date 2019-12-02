#include <iod/metajson/metajson.hh>
#include <iostream>
#include <string>

IOD_SYMBOL(age)
IOD_SYMBOL(entry)
IOD_SYMBOL(id)




int main ()
{
  using namespace iod;

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
  //struct { int age; std::string name() { return "Bob"; } } gm;
  json_str = json_object(s::age, s::name).encode(obj);
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
  iod::json_encode(std::variant<int,std::string>{"abc"});
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


  std::cout << json_object(s::age, s::name(iod::json_key("last_name"))).encode(obj) << std::endl;
  // {"age":12,"last_name":"John"}

}
