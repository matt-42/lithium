#pragma once

#include <deque>


struct pool {

  unsigned int max_size_ = 0;
  std::deque<T> pool_;
};

template <typename T>
struct pooled {

  pool<T>& pool_;
  int holder_count_ = 0;
  T t;
};
