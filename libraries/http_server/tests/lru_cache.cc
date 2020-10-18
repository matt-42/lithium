#include <lithium_http_server.hh>
#include "symbols.hh"

using namespace li;

int main() {

  lru_cache<int, int> cache(10);

  assert(12 == cache(1, [] { return 12; }));
  assert(cache.size() == 1);
  assert(12 == cache(1, [] { assert(0); return 12; }));
  assert(cache.size() == 1);

  cache.clear();
  assert(cache.size() == 0);

  for (int i = 0; i < 10; i++)
    assert(i == cache(i, [i] { return i; }));

  assert(cache.size() == 10);
  for (int i = 0; i < 10; i++)
    assert(i == cache(i, [] { assert(0); return -1; }));

  // Must evict the key 0.
  assert(42 == cache(42, [] { return 42; }));

  assert(42 == cache(0, [] { return 42; }));
}
