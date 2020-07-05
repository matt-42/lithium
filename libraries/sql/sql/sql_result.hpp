#pragma once

template <typename B>
template <typename T1, typename... T>
bool sql_result<B>::read(T1&& t1, T...& tail) {

  // Metamap and tuples
  if constexpr (li::is_metamap<T1>::value || li::is_tuple<T1>::value)
  {
    static_assert(sizeof...(T) == 0);
    return impl_->read(std::forward<T1>(t1));
  }
  // Scalar values.
  else
    return impl_->read(std::tie(t1, tail...));
}

template <typename B> template <typename T> T sql_result<B>::read() {
  T t;
  if (!this->read(t))
    throw std::runtime_error("Trying to read a request that did not return any data.");
  return t;
}

template <typename B> template <typename T> void sql_result<B>::read(std::optional<T>& o) {
  o = this->read_optional<T>();
}

template <typename B> template <typename T> std::optional<T> sql_result<B>::read_optional() {
  T t;
  if (this->read(t))
    return std::optional<T>{t};
  else
    return std::optional<T>{};
}

template <typename B> template <typename F> void sql_result<B>::map(F map_function) {

  typedef typename unconstref_tuple_elements<callable_arguments_tuple_t<F>>::ret TP;

  using A = decltype([]() {
    if constexpr (std::tuple_size<TP>::value == 1)
      return T{}; // Simgle argument.
    else if constexpr (std::tuple_size<TP>::value > 1)
      return std::tuple_element_t<0, TP>{}; // A tuple of arguments.
    else
      static_assert(0 && "map function must take at least 1 argument.");
  }());

  while (auto res = this->read_optional<A>()) {
    if constexpr (std::tuple_size<TP>::value == 1)
      map_function(res.value());
    else if constexpr (std::tuple_size<TP>::value > 1)
      std::apply(map_function, res.value());
  }
}
