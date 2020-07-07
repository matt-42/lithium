#pragma once

#include <tuple>

namespace li {
template <typename T> struct is_tuple_after_decay : std::false_type {};
template <typename... T> struct is_tuple_after_decay<std::tuple<T...>> : std::true_type {};

template <typename T> struct is_tuple : is_tuple_after_decay<std::decay_t<T>> {};
template <typename T> struct unconstref_tuple_elements {};
template <typename... T> struct unconstref_tuple_elements<std::tuple<T...>> {
  typedef std::tuple<std::remove_const_t<std::remove_reference_t<T>>...> ret;
};

} // namespace li