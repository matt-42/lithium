#include <li/metamap/make.hh>
#include <li/metamap/algorithms/cat.hh>
#include <cassert>

LI_SYMBOL(test1)
LI_SYMBOL(test2)
LI_SYMBOL(test3)

using namespace li;

int main()
{

  auto a = mmm(s::test1 = 12, s::test2 = 13);
  auto b = mmm(s::test3 = 14);

  auto c = cat(a, b);

  assert(c.test1 == 12);
  assert(c.test2 == 13);
  assert(c.test3 == 14);
}
