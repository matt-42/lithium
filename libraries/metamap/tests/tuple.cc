#include <li/metamap/metamap.hh>
#include <cassert>
#include <iostream>

int main()
{
  std::tuple<int, float, int> x{2, 3.2f, 1};

  assert(std::get<0>(li::tuple_filter<float>(x)) == 2);
  assert(std::get<1>(li::tuple_filter<float>(x)) == 1);
  assert(std::get<0>(li::tuple_filter<int>(x)) == 3.2f);
}
