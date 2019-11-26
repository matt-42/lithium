#include <iod/symbol/symbol.hh>
#include <cassert>
#include <cstring>

IOD_SYMBOL(my_symbol)

using namespace iod;

int main()
{
  

  // Simple variable.
  auto v = make_variable(s::my_symbol, 42);
  assert(v.my_symbol == 42);
  assert(!strcmp(symbol_string(v), "my_symbol"));

  // Member access.
  assert(symbol_member_access(v, s::my_symbol) == 42);  


  // Ref
  int x = 23;
  auto v2 = make_variable_reference(s::my_symbol, x);
  x++;
  assert(v2.my_symbol == 24);

  // Not ref
  int y = 23;
  auto v3 = make_variable<int>(s::my_symbol, y);
  y++;
  assert(v3.my_symbol == 23);


  // Method call
  struct {
    int my_symbol(int a) { return x + a; }
    int x;
  } obj{40};

  assert(symbol_method_call(obj, s::my_symbol, 2) == 42);
}
