#include <li/metamap/metamap.hh>
#include <cassert>
#include <string>
#include "symbols.hh"

using namespace li;


int main()
{

  // Simple map.
  auto m = mmm(s::test1 = 12, s::test2 = 13);

  assert(m.test1 == 12);
  assert(m[s::test1] == 12);
  assert(m.test2 == 13);
  assert(m[s::test2] == 13);

  if constexpr(has_key(m, s::test3)) { assert(0); }
  if constexpr(!has_key(m, s::test1)) { assert(0); }
  if constexpr(!has_key(m, s::test2)) { assert(0); }


  // References.
  int x = 41;
  auto m2 = make_metamap_reference(s::test1 = x);

  assert(m2[s::test1] == 41);
  x++;
  assert(m2[s::test1] == 42);

  auto m3 = mmm(s::test1 = std::string("test"));
  assert(m3.test1 == "test");

  // Copy.
  decltype(m3) m4 = m3;
  assert(m4.test1 == "test");
  assert(m4.test1.data() != m3.test1.data());
}
