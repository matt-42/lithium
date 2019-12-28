#pragma once

#include <vector>

namespace li {

template <typename V>
struct type_hashmap {

  template <typename... T> V& operator()(T&&...)
  {
    static int hash = values.size();
    values.resize(hash+1);
    return values[hash];
  }

private:
  std::vector<V> values;
};

}
