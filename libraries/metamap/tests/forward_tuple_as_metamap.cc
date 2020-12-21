#include <cassert>
#include <lithium_metamap.hh>

#include <string>

#include "symbols.hh"

using namespace li;

int main() {
  auto t = std::make_tuple(42, std::string("a"));

  auto m = forward_tuple_as_metamap(std::make_tuple(s::a, s::b), t);

  assert(m.a == 42);  
  assert(m.b == "a");

  m.a = 2;
  assert(std::get<0>(t) == 2);
  m.b = "bb";
  assert(std::get<1>(t) == "bb");
}
