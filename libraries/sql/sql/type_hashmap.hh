#pragma once

#include <vector>

namespace li {

template <typename V>
struct type_hashmap {

  template <typename... T> V& operator()(T&&...)
  {
    static int hash = -1;
    if (hash == -1)
    {
      std::lock_guard lock(mutex_);
      if (hash == -1)
        hash = counter_++;
    }
    values_.resize(hash+1);
    return values_[hash];
  }

private:
  static std::mutex mutex_;
  static int counter_;
  std::vector<V> values_;
};

template <typename V>
std::mutex type_hashmap<V>::mutex_;
template <typename V>
int type_hashmap<V>::counter_ = 0;

}
