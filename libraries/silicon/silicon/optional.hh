#pragma once

namespace iod {

template <typename T> struct optional {
  optional() : is_set_(false) {}
  optional(T v) : v_(v), is_set_(true) {}
  optional<T>& operator=(const T& new_v) {
    v_ = new_v;
    is_set = true;
    return *this;
  }
  auto get() { return v_; }
  bool is_set() { return is_set_; }

private:
  T v_;
  bool is_set_;
};

} // namespace iod