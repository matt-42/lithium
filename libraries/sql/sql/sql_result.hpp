#pragma once

#include <li/sql/internal/utils.hh>

namespace li {
template <typename B>
template <typename T1, typename... T>
bool sql_result<B>::read(T1&& t1, T&... tail) {

  // Metamap and tuples
  if constexpr (li::is_metamap<std::decay_t<T1>>::value || li::is_tuple<std::decay_t<T1>>::value) {
    static_assert(sizeof...(T) == 0);
    return impl_.read(std::forward<T1>(t1));
  }
  // Scalar values.
  else
    return impl_.read(std::tie(t1, tail...));
}

template <typename B> template <typename T1, typename... T> auto sql_result<B>::read() {
  auto t = [] {
    if constexpr (sizeof...(T) == 0)
      return T1{};
    else
      return std::tuple<T1, T...>{};
  }();
  if (!this->read(t))
    throw std::runtime_error("Trying to read a request that did not return any data.");
  return t;
}

template <typename B> template <typename T> void sql_result<B>::read(std::optional<T>& o) {
  o = this->read_optional<T>();
}

template <typename B>
template <typename T1, typename... T>
auto sql_result<B>::read_optional() {
  auto t = [] {
    if constexpr (sizeof...(T) == 0)
      return T1{};
    else
      return std::tuple<T1, T...>{};
  }();
  if (this->read(t))
    return std::make_optional(std::move(t));
  else
    return std::optional<decltype(t)>{};
}

namespace internal {

  template<typename T, typename F>
  constexpr auto is_valid(F&& f) -> decltype(f(std::declval<T>()), true) { return true; }

  template<typename>
  constexpr bool is_valid(...) { return false; }

}

#define IS_VALID(T, EXPR) internal::is_valid<T>( [](auto&& obj)->decltype(obj.EXPR){} )

template <typename B> template <typename F> void sql_result<B>::map(F map_function) {


  if constexpr (IS_VALID(B, map(map_function)))
    this->impl_.map(map_function);

  typedef typename unconstref_tuple_elements<callable_arguments_tuple_t<F>>::ret TP;

  auto t = [] {
    static_assert(std::tuple_size_v<TP> > 0, "sql_result map function must take at least 1 argument.");

    if constexpr (std::tuple_size_v<TP> == 1)
      return std::tuple_element_t<0, TP>{};
    else if constexpr (std::tuple_size_v<TP> > 1)
      return TP{};
  }();

  while (this->read(t)) {
    if constexpr (std::tuple_size<TP>::value == 1)
      map_function(t);
    else if constexpr (std::tuple_size<TP>::value > 1)
      std::apply(map_function, t);
  }

}
} // namespace li