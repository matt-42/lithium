#pragma once

#include <tuple>

namespace li {

constexpr int count_first_falses() { return 0; }

template <typename... B> constexpr int count_first_falses(bool b1, B... b) {
  if (b1)
    return 0;
  else
    return 1 + count_first_falses(b...);
}

template <typename E, typename... T> decltype(auto) arg_get_by_type_(void*, E* a1, T&&... args) {
  return std::forward<E*>(a1);
}

template <typename E, typename... T>
decltype(auto) arg_get_by_type_(void*, const E* a1, T&&... args) {
  return std::forward<const E*>(a1);
}

template <typename E, typename... T> decltype(auto) arg_get_by_type_(void*, E& a1, T&&... args) {
  return std::forward<E&>(a1);
}

template <typename E, typename... T>
decltype(auto) arg_get_by_type_(void*, const E& a1, T&&... args) {
  return std::forward<const E&>(a1);
}

template <typename E, typename T1, typename... T>
decltype(auto) arg_get_by_type_(std::enable_if_t<!std::is_same<E, std::decay_t<T1>>::value>*,
                                // void*,
                                T1&&, T&&... args) {
  return arg_get_by_type_<E>((void*)0, std::forward<T>(args)...);
}

template <typename E, typename... T> decltype(auto) arg_get_by_type(T&&... args) {
  return arg_get_by_type_<std::decay_t<E>>(0, args...);
}

template <typename E, typename... T> decltype(auto) tuple_get_by_type(std::tuple<T...>& tuple) {
  typedef std::decay_t<E> DE;
  return std::get<count_first_falses((std::is_same<std::decay_t<T>, DE>::value)...)>(tuple);
}

template <typename E, typename... T> decltype(auto) tuple_get_by_type(std::tuple<T...>&& tuple) {
  typedef std::decay_t<E> DE;
  return std::get<count_first_falses((std::is_same<std::decay_t<T>, DE>::value)...)>(tuple);
}

template <typename T, typename U> struct tuple_embeds : public std::false_type {};

template <typename... T, typename U>
struct tuple_embeds<std::tuple<T...>, U>
    : public std::integral_constant<bool, count_first_falses(std::is_same<T, U>::value...) !=
                                              sizeof...(T)> {};

template <typename U, typename... T> struct tuple_embeds_any_ref_of : public std::false_type {};
template <typename U, typename... T>
struct tuple_embeds_any_ref_of<std::tuple<T...>, U>
    : public tuple_embeds<std::tuple<std::decay_t<T>...>, std::decay_t<U>> {};

template <typename T> struct tuple_remove_references;
template <typename... T> struct tuple_remove_references<std::tuple<T...>> {
  typedef std::tuple<std::remove_reference_t<T>...> type;
};

template <typename T> using tuple_remove_references_t = typename tuple_remove_references<T>::type;

template <typename T> struct tuple_remove_references_and_const;
template <typename... T> struct tuple_remove_references_and_const<std::tuple<T...>> {
  typedef std::tuple<std::remove_const_t<std::remove_reference_t<T>>...> type;
};

template <typename T>
using tuple_remove_references_and_const_t = typename tuple_remove_references_and_const<T>::type;

template <typename T, typename U, typename E> struct tuple_remove_element2;

template <typename... T, typename... U, typename E1>
struct tuple_remove_element2<std::tuple<E1, T...>, std::tuple<U...>, E1>
    : public tuple_remove_element2<std::tuple<T...>, std::tuple<U...>, E1> {};

template <typename... T, typename... U, typename T1, typename E1>
struct tuple_remove_element2<std::tuple<T1, T...>, std::tuple<U...>, E1>
    : public tuple_remove_element2<std::tuple<T...>, std::tuple<U..., T1>, E1> {};

template <typename... U, typename E1>
struct tuple_remove_element2<std::tuple<>, std::tuple<U...>, E1> {
  typedef std::tuple<U...> type;
};

template <typename T, typename E>
struct tuple_remove_element : public tuple_remove_element2<T, std::tuple<>, E> {};

template <typename T, typename... E> struct tuple_remove_elements;

template <typename... T, typename E1, typename... E>
struct tuple_remove_elements<std::tuple<T...>, E1, E...> {
  typedef typename tuple_remove_elements<typename tuple_remove_element<std::tuple<T...>, E1>::type,
                                         E...>::type type;
};

template <typename... T> struct tuple_remove_elements<std::tuple<T...>> {
  typedef std::tuple<T...> type;
};

template <typename A, typename B> struct tuple_minus;

template <typename... T, typename... R> struct tuple_minus<std::tuple<T...>, std::tuple<R...>> {
  typedef typename tuple_remove_elements<std::tuple<T...>, R...>::type type;
};

template <typename T, typename... E>
using tuple_remove_elements_t = typename tuple_remove_elements<T, E...>::type;

template <typename F, size_t... I, typename... T>
inline F tuple_map(std::tuple<T...>& t, F f, std::index_sequence<I...>) {
  return (void)std::initializer_list<int>{((void)f(std::get<I>(t)), 0)...}, f;
}

template <typename F, typename... T> inline void tuple_map(std::tuple<T...>& t, F f) {
  tuple_map(t, f, std::index_sequence_for<T...>{});
}

template <typename F, size_t... I, typename T>
inline decltype(auto) tuple_transform(T&& t, F f, std::index_sequence<I...>) {
  return std::make_tuple(f(std::get<I>(std::forward<T>(t)))...);
}

template <typename F, typename T> inline decltype(auto) tuple_transform(T&& t, F f) {
  return tuple_transform(std::forward<T>(t), f,
                         std::make_index_sequence<std::tuple_size<std::decay_t<T>>{}>{});
}

template <template <class> class F, typename T, typename I, typename R, typename X = void>
struct tuple_filter_sequence;

template <template <class> class F, typename... T, typename R>
struct tuple_filter_sequence<F, std::tuple<T...>, std::index_sequence<>, R> {
  using ret = R;
};

template <template <class> class F, typename T1, typename... T, size_t I1, size_t... I, size_t... R>
struct tuple_filter_sequence<F, std::tuple<T1, T...>, std::index_sequence<I1, I...>,
                             std::index_sequence<R...>, std::enable_if_t<F<T1>::value>> {
  using ret = typename tuple_filter_sequence<F, std::tuple<T...>, std::index_sequence<I...>,
                                             std::index_sequence<R..., I1>>::ret;
};

template <template <class> class F, typename T1, typename... T, size_t I1, size_t... I, size_t... R>
struct tuple_filter_sequence<F, std::tuple<T1, T...>, std::index_sequence<I1, I...>,
                             std::index_sequence<R...>, std::enable_if_t<!F<T1>::value>> {
  using ret = typename tuple_filter_sequence<F, std::tuple<T...>, std::index_sequence<I...>,
                                             std::index_sequence<R...>>::ret;
};

template <std::size_t... I, typename T>
decltype(auto) tuple_filter_impl(std::index_sequence<I...>, T&& t) {
  return std::make_tuple(std::get<I>(t)...);
}

template <template <class> class F, typename T> decltype(auto) tuple_filter(T&& t) {
  using seq = typename tuple_filter_sequence<
      F, std::decay_t<T>, std::make_index_sequence<std::tuple_size<std::decay_t<T>>::value>,
      std::index_sequence<>>::ret;
  return tuple_filter_impl(seq{}, t);
}
} // namespace li
