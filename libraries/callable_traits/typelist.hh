#pragma once

#include <li/metamap/algorithms/tuple_utils.hh>
#include <tuple>

namespace li {

template <typename... T> struct typelist {};

template <typename... T1, typename... T2>
constexpr auto typelist_cat(typelist<T1...> t1, typelist<T2...> t2) {
  return typelist<T1..., T2...>();
}

template <typename T> struct typelist_to_tuple {};

template <typename... T> struct typelist_to_tuple<typelist<T...>> {
  typedef std::tuple<T...> type;
};

template <typename T> struct tuple_to_typelist {};

template <typename... T> struct tuple_to_typelist<std::tuple<T...>> {
  typedef typelist<T...> type;
};

template <typename T> using typelist_to_tuple_t = typename typelist_to_tuple<T>::type;
template <typename T> using tuple_to_typelist_t = typename tuple_to_typelist<T>::type;

template <typename T, typename U> struct typelist_embeds : public std::false_type {};

template <typename... T, typename U>
struct typelist_embeds<typelist<T...>, U>
    : public std::integral_constant<bool, count_first_falses(std::is_same<T, U>::value...) !=
                                              sizeof...(T)> {};

template <typename T, typename E> struct typelist_embeds_any_ref_of : public std::false_type {};

template <typename U, typename... T>
struct typelist_embeds_any_ref_of<typelist<T...>, U>
    : public typelist_embeds<typelist<std::decay_t<T>...>, std::decay_t<U>> {};

} // namespace li
