#include <iod/metamap/make.hh>
#include <iod/metamap/algorithms/intersection.hh>
#include <cassert>
#include <string>

IOD_SYMBOL(test1)
IOD_SYMBOL(test2)
IOD_SYMBOL(test3)
IOD_SYMBOL(test4)
IOD_SYMBOL(test5)

using namespace iod;

int main()
{
  auto a = make_metamap(s::test1 = 12, s::test2 = 13, s::test4 = 14, s::test5 = std::string("test"));

  auto b = make_metamap(s::test2 = 12, s::test3 = 14, s::test5 = 16);
  auto c = intersection(a, b);

  assert(!has_key(c, s::test1));
  assert(has_key(c, s::test2));
  assert(!has_key(c, s::test3));
  assert(!has_key(c, s::test4));
  assert(has_key(c, s::test5));

  assert(c.test2 == 13);
  assert(c.test5 == "test");
}
