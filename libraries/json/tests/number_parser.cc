#include <cassert>
#include <iomanip>
#include <iostream>
#include <lithium_json.hh>

int test_integer(const char* str) {
  int f;
  const char* end;
  li::internal::parse_int(&f, str, &end);

  std::cout << f << std::endl;
  assert((end - str) == strlen(str));
  return f;
}

double test_float(const char* str) {
  double f;
  const char* end;
  li::internal::parse_float(&f, str, &end);
  std::cout << std::string(str, end) << " -> " << std::setprecision(20) << std::fixed << f
            << std::endl;
  assert((end - str) == strlen(str));
  return f;
}

#define TEST(N) assert(test_integer(#N) == N);
#define TESTF(N) assert(std::abs(test_float(#N) - N) < 1e-5);

int main() {
  TEST(0);
  TEST(1);
  TEST(-1);
  TEST(123456789);
  TEST(0001);

  TESTF(0);
  TESTF(0.0);
  TESTF(0.01);
  TESTF(1);
  TESTF(1.);
  TESTF(1.0);
  TESTF(1.01);
  TESTF(01.01);
  TESTF(1e10);
  TESTF(1e0);
  TESTF(1.21e10);
  TESTF(1.21e-10);
  TESTF(1234567890.1234567890);
  TESTF(-1234567890.1234567890);

  assert(test_float("1E-10") == test_float("1e-10"));
}
