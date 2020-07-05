#pragma once
#include <tuple>

namespace li {
template <typename T> struct is_tuple : std::false {};
template <typename... T> struct is_tuple<std::tuple<T...>> : std::true {};
template <typename T> struct unconstref_tuple_elements {};
template <typename... T> struct unconstref_tuple_elements<std::tuple<T...>> {
  typedef std::tuple<std::remove_const_t<std::remove_reference_t<T>>...> ret;
};

} // namespace li