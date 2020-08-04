#pragma once

#include <tuple>
#include <iostream>

namespace li {

inline void println() { 
  std::cout << std::endl; 
}

template <typename A, typename... T> inline void println(A &&a, T &&... args) {
  std::cout << a << " ";
  println(std::forward<T>(args)...);
}

template <typename T> struct is_tuple_after_decay : std::false_type {};
template <typename... T> struct is_tuple_after_decay<std::tuple<T...>> : std::true_type {};

template <typename T> struct is_tuple : is_tuple_after_decay<std::decay_t<T>> {};
template <typename T> struct unconstref_tuple_elements {};
template <typename... T> struct unconstref_tuple_elements<std::tuple<T...>> {
  typedef std::tuple<std::remove_const_t<std::remove_reference_t<T>>...> ret;
};

} // namespace li