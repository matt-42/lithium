// Author: Matthieu Garrigues matthieu.garrigues@gmail.com
//
// Single header version the lithium_sqlite library.
// https://github.com/matt-42/lithium
//
// This file is generated do not edit it.

#pragma once

#include <any>
#include <atomic>
#include <boost/lexical_cast.hpp>
#include <cassert>
#include <cstring>
#include <deque>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <sqlite3.h>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#if defined(_MSC_VER)
#include <ciso646>
#endif // _MSC_VER


#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQLITE_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQLITE_HH

#if defined(_MSC_VER)
#endif


#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_CALLABLE_TRAITS_CALLABLE_TRAITS_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_CALLABLE_TRAITS_CALLABLE_TRAITS_HH

namespace li {

template <typename... T> struct typelist {};

namespace internal {
template <typename T> struct has_parenthesis_operator {
  template <typename C> static char test(decltype(&C::operator()));
  template <typename C> static int test(...);
  static const bool value = sizeof(test<T>(0)) == 1;
};

} // namespace internal

// Traits on callable (function, functors and lambda functions).

// callable_traits<F>::is_callable = true_type if F is callable.
// callable_traits<F>::arity = N if F takes N arguments.
// callable_traits<F>::arguments_tuple_type = tuple<Arg1, ..., ArgN>

template <typename F, typename X = void> struct callable_traits {
  typedef std::false_type is_callable;
  static const int arity = 0;
  typedef std::tuple<> arguments_tuple;
  typedef typelist<> arguments_list;
  typedef void return_type;
};

template <typename F, typename X> struct callable_traits<F&, X> : public callable_traits<F, X> {};
template <typename F, typename X> struct callable_traits<F&&, X> : public callable_traits<F, X> {};
template <typename F, typename X>
struct callable_traits<const F&, X> : public callable_traits<F, X> {};

template <typename F>
struct callable_traits<F, std::enable_if_t<internal::has_parenthesis_operator<F>::value>> {
  typedef callable_traits<decltype(&F::operator())> super;
  typedef std::true_type is_callable;
  static const int arity = super::arity;
  typedef typename super::arguments_tuple arguments_tuple;
  typedef typename super::arguments_list arguments_list;
  typedef typename super::return_type return_type;
};

template <typename C, typename R, typename... ARGS>
struct callable_traits<R (C::*)(ARGS...) const> {
  typedef std::true_type is_callable;
  static const int arity = sizeof...(ARGS);
  typedef std::tuple<ARGS...> arguments_tuple;
  typedef typelist<ARGS...> arguments_list;
  typedef R return_type;
};

template <typename C, typename R, typename... ARGS> struct callable_traits<R (C::*)(ARGS...)> {
  typedef std::true_type is_callable;
  static const int arity = sizeof...(ARGS);
  typedef std::tuple<ARGS...> arguments_tuple;
  typedef typelist<ARGS...> arguments_list;
  typedef R return_type;
};

template <typename R, typename... ARGS> struct callable_traits<R(ARGS...)> {
  typedef std::true_type is_callable;
  static const int arity = sizeof...(ARGS);
  typedef std::tuple<ARGS...> arguments_tuple;
  typedef typelist<ARGS...> arguments_list;
  typedef R return_type;
};

template <typename R, typename... ARGS> struct callable_traits<R (*)(ARGS...)> {
  typedef std::true_type is_callable;
  static const int arity = sizeof...(ARGS);
  typedef std::tuple<ARGS...> arguments_tuple;
  typedef typelist<ARGS...> arguments_list;
  typedef R return_type;
};

template <typename F>
using callable_arguments_tuple_t = typename callable_traits<F>::arguments_tuple;
template <typename F> using callable_arguments_list_t = typename callable_traits<F>::arguments_list;
template <typename F> using callable_return_type_t = typename callable_traits<F>::return_type;

template <typename F> struct is_callable : public callable_traits<F>::is_callable {};

template <typename F, typename... A> struct callable_with {
  template <typename G, typename... B>
  static char test(int x,
                   std::remove_reference_t<decltype(std::declval<G>()(std::declval<B>()...))>* = 0);
  template <typename G, typename... B> static int test(...);
  static const bool value = sizeof(test<F, A...>(0)) == 1;
};

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_CALLABLE_TRAITS_CALLABLE_TRAITS_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_CALLABLE_TRAITS_TUPLE_UTILS_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_CALLABLE_TRAITS_TUPLE_UTILS_HH


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

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_CALLABLE_TRAITS_TUPLE_UTILS_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_METAMAP_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_METAMAP_HH


namespace li {

namespace internal {
struct {
  template <typename A, typename... B> constexpr auto operator()(A&& a, B&&... b) {
    auto result = a;
    using expand_variadic_pack = int[];
    (void)expand_variadic_pack{0, ((result += b), 0)...};
    return result;
  }
} reduce_add;

} // namespace internal

template <typename... Ms> struct metamap;

template <typename F, typename... M> constexpr decltype(auto) find_first(metamap<M...>&& map, F fun);

template <typename... Ms> struct metamap;

template <typename M1, typename... Ms> struct metamap<M1, Ms...> : public M1, public Ms... {
  typedef metamap<M1, Ms...> self;
  // Constructors.
  inline metamap() = default;
  inline metamap(self&&) = default;
  inline metamap(const self&) = default;
  self& operator=(const self&) = default;
  self& operator=(self&&) = default;

  // metamap(self& other)
  //  : metamap(const_cast<const self&>(other)) {}

  constexpr inline metamap(typename M1::_iod_value_type&& m1, typename Ms::_iod_value_type&&... members) : M1{m1}, Ms{std::forward<typename Ms::_iod_value_type>(members)}... {}
  constexpr inline metamap(M1&& m1, Ms&&... members) : M1(m1), Ms(std::forward<Ms>(members))... {}
  constexpr inline metamap(const M1& m1, const Ms&... members) : M1(m1), Ms((members))... {}

  // Assignemnt ?

  // Retrive a value.
  template <typename K> constexpr decltype(auto) operator[](K k) { return symbol_member_access(*this, k); }

  template <typename K> constexpr decltype(auto) operator[](K k) const {
    return symbol_member_access(*this, k);
  }
};

template <> struct metamap<> {
  typedef metamap<> self;
  // Constructors.
  constexpr inline metamap() = default;
  // inline metamap(self&&) = default;
  constexpr inline metamap(const self&) = default;
  // self& operator=(const self&) = default;

  // metamap(self& other)
  //  : metamap(const_cast<const self&>(other)) {}

  // Assignemnt ?

  // Retrive a value.
  template <typename K> constexpr decltype(auto) operator[](K k) { return symbol_member_access(*this, k); }

  template <typename K> constexpr decltype(auto) operator[](K k) const {
    return symbol_member_access(*this, k);
  }
};

template <typename... Ms> constexpr auto size(metamap<Ms...>) { return sizeof...(Ms); }

template <typename M> struct metamap_size_t {};
template <typename... Ms> struct metamap_size_t<metamap<Ms...>> {
  enum { value = sizeof...(Ms) };
};
template <typename M> constexpr int metamap_size() {
  return metamap_size_t<std::decay_t<M>>::value;
}

template <typename... Ks> constexpr decltype(auto) metamap_values(const metamap<Ks...>& map) {
  return std::forward_as_tuple(map[typename Ks::_iod_symbol_type()]...);
}
template <typename... Ks> constexpr decltype(auto) metamap_values(metamap<Ks...>& map) {
  return std::forward_as_tuple(map[typename Ks::_iod_symbol_type()]...);
}

template <typename... Ks> constexpr decltype(auto) metamap_keys(const metamap<Ks...>& map) {
  return std::make_tuple(typename Ks::_iod_symbol_type()...);
}

template <typename K, typename M> constexpr auto has_key(M&& map, K k) {
  return decltype(has_member(map, k)){};
}

template <typename M, typename K> constexpr auto has_key(K k) {
  return decltype(has_member(std::declval<M>(), std::declval<K>())){};
}

template <typename M, typename K> constexpr auto has_key() {
  return decltype(has_member(std::declval<M>(), std::declval<K>())){};
}

template <typename K, typename M, typename O> constexpr auto get_or(M&& map, K k, O default_) {
  if constexpr (has_key<M, decltype(k)>()) {
    return map[k];
  } else
    return default_;
}

template <typename X> struct is_metamap {
  enum { value = false };
};
template <typename... M> struct is_metamap<metamap<M...>> {
  enum { value = true };
};

} // namespace li

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_CAT_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_CAT_HH

namespace li {

template <typename... T, typename... U>
constexpr inline decltype(auto) cat(const metamap<T...>& a, const metamap<U...>& b) {
  return metamap<T..., U...>(*static_cast<const T*>(&a)..., *static_cast<const U*>(&b)...);
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_CAT_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_INTERSECTION_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_INTERSECTION_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_MAKE_METAMAP_SKIP_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_MAKE_METAMAP_SKIP_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_MAKE_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_MAKE_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SYMBOL_AST_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SYMBOL_AST_HH


namespace li {

template <typename E> struct Exp {};

template <typename E> struct array_subscriptable;

template <typename E> struct callable;

template <typename E> struct assignable;

template <typename E> struct array_subscriptable;

template <typename M, typename... A>
struct function_call_exp : public array_subscriptable<function_call_exp<M, A...>>,
                           public callable<function_call_exp<M, A...>>,
                           public assignable<function_call_exp<M, A...>>,
                           public Exp<function_call_exp<M, A...>> {
  using assignable<function_call_exp<M, A...>>::operator=;

  function_call_exp(const M& m, A&&... a) : method(m), args(std::forward<A>(a)...) {}

  M method;
  std::tuple<A...> args;
};

template <typename O, typename M>
struct array_subscript_exp : public array_subscriptable<array_subscript_exp<O, M>>,
                             public callable<array_subscript_exp<O, M>>,
                             public assignable<array_subscript_exp<O, M>>,
                             public Exp<array_subscript_exp<O, M>> {
  using assignable<array_subscript_exp<O, M>>::operator=;

  array_subscript_exp(const O& o, const M& m) : object(o), member(m) {}

  O object;
  M member;
};

template <typename L, typename R> struct assign_exp : public Exp<assign_exp<L, R>> {
  typedef L left_t;
  typedef R right_t;

  // template <typename V>
  // assign_exp(L l, V&& r) : left(l), right(std::forward<V>(r)) {}
  // template <typename V>
  inline assign_exp(L l, R r) : left(l), right(r) {}
  // template <typename V>
  // inline assign_exp(L l, const V& r) : left(l), right(r) {}

  L left;
  R right;
};

template <typename E> struct array_subscriptable {
public:
  // Member accessor
  template <typename S> constexpr auto operator[](S&& s) const {
    return array_subscript_exp<E, S>(*static_cast<const E*>(this), std::forward<S>(s));
  }
};

template <typename E> struct callable {
public:
  // Direct call.
  template <typename... A> constexpr auto operator()(A&&... args) const {
    return function_call_exp<E, A...>(*static_cast<const E*>(this), std::forward<A>(args)...);
  }
};

template <typename E> struct assignable {
public:
  template <typename L> auto operator=(L&& l) const {
    return assign_exp<E, L>(static_cast<const E&>(*this), std::forward<L>(l));
  }

  template <typename L> auto operator=(L&& l) {
    return assign_exp<E, L>(static_cast<E&>(*this), std::forward<L>(l));
  }

  template <typename T> auto operator=(const std::initializer_list<T>& l) const {
    return assign_exp<E, std::vector<T>>(static_cast<const E&>(*this), std::vector<T>(l));
  }
};

#define iod_query_declare_binary_op(OP, NAME)                                                      \
  template <typename A, typename B>                                                                \
  struct NAME##_exp : public assignable<NAME##_exp<A, B>>, public Exp<NAME##_exp<A, B>> {          \
    using assignable<NAME##_exp<A, B>>::operator=;                                                 \
    NAME##_exp() {}                                                                                \
    NAME##_exp(A&& a, B&& b) : lhs(std::forward<A>(a)), rhs(std::forward<B>(b)) {}                 \
    typedef A lhs_type;                                                                            \
    typedef B rhs_type;                                                                            \
    lhs_type lhs;                                                                                  \
    rhs_type rhs;                                                                                  \
  };                                                                                               \
  template <typename A, typename B>                                                                \
  inline std::enable_if_t<std::is_base_of<Exp<A>, A>::value || std::is_base_of<Exp<B>, B>::value,  \
                          NAME##_exp<A, B>>                                                        \
  operator OP(const A& b, const B& a) {                                                            \
    return NAME##_exp<std::decay_t<A>, std::decay_t<B>>{b, a};                                     \
  }

iod_query_declare_binary_op(+, plus);
iod_query_declare_binary_op(-, minus);
iod_query_declare_binary_op(*, mult);
iod_query_declare_binary_op(/, div);
iod_query_declare_binary_op(<<, shiftl);
iod_query_declare_binary_op(>>, shiftr);
iod_query_declare_binary_op(<, inf);
iod_query_declare_binary_op(<=, inf_eq);
iod_query_declare_binary_op(>, sup);
iod_query_declare_binary_op(>=, sup_eq);
iod_query_declare_binary_op(==, eq);
iod_query_declare_binary_op(!=, neq);
iod_query_declare_binary_op(&, logical_and);
iod_query_declare_binary_op (^, logical_xor);
iod_query_declare_binary_op(|, logical_or);
iod_query_declare_binary_op(&&, and);
iod_query_declare_binary_op(||, or);

#undef iod_query_declare_binary_op

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SYMBOL_AST_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SYMBOL_SYMBOL_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SYMBOL_SYMBOL_HH


namespace li {

template <typename S>
class symbol : public assignable<S>,
               public array_subscriptable<S>,
               public callable<S>,
               public Exp<S> {};
} // namespace li

#ifdef LI_SYMBOL
#undef LI_SYMBOL
#endif

#define LI_SYMBOL(NAME)                                                                            \
  namespace s {                                                                                    \
  struct NAME##_t : li::symbol<NAME##_t> {                                                         \
                                                                                                   \
    using assignable<NAME##_t>::operator=;                                                         \
                                                                                                   \
    inline constexpr bool operator==(NAME##_t) { return true; }                                    \
    template <typename T> inline constexpr bool operator==(T) { return false; }                    \
                                                                                                   \
    template <typename V> struct variable_t {                                                      \
      typedef NAME##_t _iod_symbol_type;                                                           \
      typedef V _iod_value_type;                                                                   \
      V NAME;                                                                                      \
    };                                                                                             \
                                                                                                   \
    template <typename T, typename... A>                                                           \
    static inline decltype(auto) symbol_method_call(T&& o, A... args) {                            \
      return o.NAME(args...);                                                                      \
    }                                                                                              \
    template <typename T, typename... A> static inline auto& symbol_member_access(T&& o) {         \
      return o.NAME;                                                                               \
    }                                                                                              \
    template <typename T>                                                                          \
    static constexpr auto has_getter(int)                                                          \
        -> decltype(std::declval<T>().NAME(), std::true_type{}) {                                  \
      return {};                                                                                   \
    }                                                                                              \
    template <typename T> static constexpr auto has_getter(long) { return std::false_type{}; }     \
    template <typename T>                                                                          \
    static constexpr auto has_member(int) -> decltype(std::declval<T>().NAME, std::true_type{}) {  \
      return {};                                                                                   \
    }                                                                                              \
    template <typename T> static constexpr auto has_member(long) { return std::false_type{}; }     \
                                                                                                   \
    static inline auto symbol_string() { return #NAME; }                                           \
  };                                                                                               \
  static constexpr NAME##_t NAME;                                                                  \
  }

namespace li {

template <typename S> inline decltype(auto) make_variable(S s, char const v[]) {
  typedef typename S::template variable_t<const char*> ret;
  return ret{v};
}

template <typename V, typename S> inline decltype(auto) make_variable(S s, V v) {
  typedef typename S::template variable_t<std::remove_const_t<std::remove_reference_t<V>>> ret;
  return ret{v};
}

template <typename K, typename V> inline decltype(auto) make_variable_reference(K s, V&& v) {
  typedef typename K::template variable_t<V> ret;
  return ret{v};
}

template <typename T, typename S, typename... A>
static inline decltype(auto) symbol_method_call(T&& o, S, A... args) {
  return S::symbol_method_call(o, std::forward<A>(args)...);
}

template <typename T, typename S> static inline decltype(auto) symbol_member_access(T&& o, S) {
  return S::symbol_member_access(o);
}

template <typename T, typename S> constexpr auto has_member(T&& o, S) {
  return S::template has_member<T>(0);
}
template <typename T, typename S> constexpr auto has_member() {
  return S::template has_member<T>(0);
}

template <typename T, typename S> constexpr auto has_getter(T&& o, S) {
  return decltype(S::template has_getter<T>(0)){};
}
template <typename T, typename S> constexpr auto has_getter() {
  return decltype(S::template has_getter<T>(0)){};
}

template <typename S, typename T> struct CANNOT_FIND_REQUESTED_MEMBER_IN_TYPE {};

template <typename T, typename S> decltype(auto) symbol_member_or_getter_access(T&& o, S) {
  if constexpr (has_getter<T, S>()) {
    return symbol_method_call(o, S{});
  } else if constexpr (has_member<T, S>()) {
    return symbol_member_access(o, S{});
  } else {
    return CANNOT_FIND_REQUESTED_MEMBER_IN_TYPE<S, T>::error;
  }
}

template <typename S> auto symbol_string(symbol<S> v) { return S::symbol_string(); }

template <typename V> auto symbol_string(V v, typename V::_iod_symbol_type* = 0) {
  return V::_iod_symbol_type::symbol_string();
}
} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SYMBOL_SYMBOL_HH


namespace li {

template <typename... Ms> struct metamap;

namespace internal {

template <typename S, typename V> constexpr decltype(auto) exp_to_variable_ref(const assign_exp<S, V>& e) {
  return make_variable_reference(S{}, e.right);
}

template <typename S, typename V> constexpr decltype(auto) exp_to_variable(const assign_exp<S, V>& e) {
  typedef std::remove_const_t<std::remove_reference_t<V>> vtype;
  return make_variable(S{}, e.right);
}

template <typename S> decltype(auto) constexpr exp_to_variable(const symbol<S>& e) {
  return exp_to_variable(S() = int());
}

template <typename... T> constexpr inline decltype(auto) make_metamap_helper(T&&... args) {
  return metamap<T...>(std::forward<T>(args)...);
}

} // namespace internal

// Store copies of values in the map
template <typename... T> constexpr inline decltype(auto) mmm(T&&... args) {
  // Copy values.
  return internal::make_metamap_helper(internal::exp_to_variable(std::forward<T>(args))...);
}

// Store references of values in the map
template <typename... T> constexpr inline decltype(auto) make_metamap_reference(T&&... args) {
  // Keep references.
  return internal::make_metamap_helper(internal::exp_to_variable_ref(std::forward<T>(args))...);
}

template <typename... Ks> constexpr decltype(auto) metamap_clone(const metamap<Ks...>& map) {
  return mmm((typename Ks::_iod_symbol_type() = map[typename Ks::_iod_symbol_type()])...);
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_MAKE_HH


namespace li {

struct skip {};
static struct {

  template <typename... M, typename... T>
  constexpr inline decltype(auto) run(metamap<M...> map, skip, T&&... args) const {
    return run(map, std::forward<T>(args)...);
  }

  template <typename T1, typename... M, typename... T>
  constexpr inline decltype(auto) run(metamap<M...> map, T1&& a, T&&... args) const {
    return run(
        cat(map, internal::make_metamap_helper(internal::exp_to_variable(std::forward<T1>(a)))),
        std::forward<T>(args)...);
  }

  template <typename... M> constexpr inline decltype(auto) run(metamap<M...> map) const { return map; }

  template <typename... T> constexpr inline decltype(auto) operator()(T&&... args) const {
    // Copy values.
    return run(metamap<>{}, std::forward<T>(args)...);
  }

} make_metamap_skip;

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_MAKE_METAMAP_SKIP_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_MAP_REDUCE_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_MAP_REDUCE_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_TUPLE_UTILS_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_TUPLE_UTILS_HH


namespace li {

template <typename... E, typename F> constexpr void apply_each(F&& f, E&&... e) {
  (void)std::initializer_list<int>{((void)f(std::forward<E>(e)), 0)...};
}

template <typename... E, typename F, typename R>
constexpr auto tuple_map_reduce_impl(F&& f, R&& reduce, E&&... e) {
  return reduce(f(std::forward<E>(e))...);
}

template <typename T, typename F> constexpr void tuple_map(T&& t, F&& f) {
  return std::apply([&](auto&&... e) { apply_each(f, std::forward<decltype(e)>(e)...); },
                    std::forward<T>(t));
}

template <typename T, typename F> constexpr auto tuple_reduce(T&& t, F&& f) {
  return std::apply(std::forward<F>(f), std::forward<T>(t));
}

template <typename T, typename F, typename R>
decltype(auto) tuple_map_reduce(T&& m, F map, R reduce) {
  auto fun = [&](auto... e) { return tuple_map_reduce_impl(map, reduce, e...); };
  return std::apply(fun, m);
}

template <typename F> constexpr inline std::tuple<> tuple_filter_impl() { return std::make_tuple(); }

template <typename F, typename... M, typename M1> constexpr auto tuple_filter_impl(M1 m1, M... m) {
  if constexpr (std::is_same<M1, F>::value)
    return tuple_filter_impl<F>(m...);
  else
    return std::tuple_cat(std::make_tuple(m1), tuple_filter_impl<F>(m...));
}

template <typename F, typename... M> constexpr auto tuple_filter(const std::tuple<M...>& m) {

  auto fun = [](auto... e) { return tuple_filter_impl<F>(e...); };
  return std::apply(fun, m);
}

} // namespace li
#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_TUPLE_UTILS_HH


namespace li {

// Map a function(key, value) on all kv pair
template <typename... M, typename F> constexpr void map(const metamap<M...>& m, F fun) {
  auto apply = [&](auto key) -> decltype(auto) { return fun(key, m[key]); };

  apply_each(apply, typename M::_iod_symbol_type{}...);
}

// Map a function(key, value) on all kv pair. Ensure that the calling order
// is kept.
// template <typename O, typename F>
// void map_sequential2(F fun, O& obj)
// {}
// template <typename O, typename M1, typename... M, typename F>
// void map_sequential2(F fun, O& obj, M1 m1, M... ms)
// {
//   auto apply = [&] (auto key) -> decltype(auto)
//     {
//       return fun(key, obj[key]);
//     };

//   apply(m1);
//   map_sequential2(fun, obj, ms...);
// }
// template <typename... M, typename F>
// void map_sequential(const metamap<M...>& m, F fun)
// {
//   auto apply = [&] (auto key) -> decltype(auto)
//     {
//       return fun(key, m[key]);
//     };

//   map_sequential2(fun, m, typename M::_iod_symbol_type{}...);
// }

// Map a function(key, value) on all kv pair (non const).
template <typename... M, typename F> constexpr void map(metamap<M...>& m, F fun) {
  auto apply = [&](auto key) -> decltype(auto) { return fun(key, m[key]); };

  apply_each(apply, typename M::_iod_symbol_type{}...);
}

template <typename... E, typename F, typename R> constexpr auto apply_each2(F&& f, R&& r, E&&... e) {
  return r(f(std::forward<E>(e))...);
  //(void)std::initializer_list<int>{
  //  ((void)f(std::forward<E>(e)), 0)...};
}

// Map a function(key, value) on all kv pair an reduce
// all the results value with the reduce(r1, r2, ...) function.
template <typename... M, typename F, typename R>
constexpr decltype(auto) map_reduce(const metamap<M...>& m, F map, R reduce) {
  auto apply = [&](auto key) -> decltype(auto) {
    // return map(key, std::forward<decltype(m[key])>(m[key]));
    return map(key, m[key]);
  };

  return apply_each2(apply, reduce, typename M::_iod_symbol_type{}...);
  // return reduce(apply(typename M::_iod_symbol_type{})...);
}

// Map a function(key, value) on all kv pair an reduce
// all the results value with the reduce(r1, r2, ...) function.
template <typename... M, typename R> constexpr decltype(auto) reduce(const metamap<M...>& m, R reduce) {
  return reduce(m[typename M::_iod_symbol_type{}]...);
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_MAP_REDUCE_HH



namespace li {

template <typename... T, typename... U>
constexpr inline decltype(auto) intersection(const metamap<T...>& a, const metamap<U...>& b) {
  return map_reduce(a,
                    [&](auto k, auto&& v) -> decltype(auto) {
                      if constexpr (has_key<metamap<U...>, decltype(k)>()) {
                        return k = std::forward<decltype(v)>(v);
                      } else
                        return skip{};
                    },
                    make_metamap_skip);
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_INTERSECTION_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_SUBSTRACT_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_SUBSTRACT_HH


namespace li {

template <typename... T, typename... U>
constexpr inline auto substract(const metamap<T...>& a, const metamap<U...>& b) {
  return map_reduce(a,
                    [&](auto k, auto&& v) {
                      if constexpr (!has_key<metamap<U...>, decltype(k)>()) {
                        return k = std::forward<decltype(v)>(v);
                      } else
                        return skip{};
                    },
                    make_metamap_skip);
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_SUBSTRACT_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_FORWARD_TUPLE_AS_METAMAP_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_FORWARD_TUPLE_AS_METAMAP_HH


namespace li {

namespace internal {

template <typename... S, typename... V, typename T, T... I>
constexpr auto forward_tuple_as_metamap_impl(std::tuple<S...> keys, std::tuple<V...>& values, std::integer_sequence<T, I...>) {
  return make_metamap_reference((std::get<I>(keys) = std::get<I>(values))...);
}
template <typename... S, typename... V, typename T, T... I>
constexpr auto forward_tuple_as_metamap_impl(std::tuple<S...> keys, const std::tuple<V...>& values, std::integer_sequence<T, I...>) {
  return make_metamap_reference((std::get<I>(keys) = std::get<I>(values))...);
}

} // namespace internal

template <typename... S, typename... V>
constexpr auto forward_tuple_as_metamap(std::tuple<S...> keys, std::tuple<V...>& values) {
  return internal::forward_tuple_as_metamap_impl(keys, values, std::make_index_sequence<sizeof...(V)>{});
}
template <typename... S, typename... V>
constexpr auto forward_tuple_as_metamap(std::tuple<S...> keys, const std::tuple<V...>& values) {
  return internal::forward_tuple_as_metamap_impl(keys, values, std::make_index_sequence<sizeof...(V)>{});
}

} // namespace li
#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_FORWARD_TUPLE_AS_METAMAP_HH


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_METAMAP_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_COMMON_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_COMMON_HH


namespace li {
struct sql_blob : public std::string {
  using std::string::string;
  using std::string::operator=;

  sql_blob() : std::string() {}
};

struct sql_null_t {};
static sql_null_t null;

template <unsigned SIZE> struct sql_varchar : public std::string {
  using std::string::string;
  using std::string::operator=;

  sql_varchar() : std::string() {}
};
} // namespace li
#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_COMMON_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_RESULT_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_RESULT_HH


#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SYMBOLS_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SYMBOLS_HH

#ifndef LI_SYMBOL_ATTR
#define LI_SYMBOL_ATTR
    LI_SYMBOL(ATTR)
#endif

#ifndef LI_SYMBOL_after_insert
#define LI_SYMBOL_after_insert
    LI_SYMBOL(after_insert)
#endif

#ifndef LI_SYMBOL_after_remove
#define LI_SYMBOL_after_remove
    LI_SYMBOL(after_remove)
#endif

#ifndef LI_SYMBOL_after_update
#define LI_SYMBOL_after_update
    LI_SYMBOL(after_update)
#endif

#ifndef LI_SYMBOL_auto_increment
#define LI_SYMBOL_auto_increment
    LI_SYMBOL(auto_increment)
#endif

#ifndef LI_SYMBOL_before_insert
#define LI_SYMBOL_before_insert
    LI_SYMBOL(before_insert)
#endif

#ifndef LI_SYMBOL_before_remove
#define LI_SYMBOL_before_remove
    LI_SYMBOL(before_remove)
#endif

#ifndef LI_SYMBOL_before_update
#define LI_SYMBOL_before_update
    LI_SYMBOL(before_update)
#endif

#ifndef LI_SYMBOL_charset
#define LI_SYMBOL_charset
    LI_SYMBOL(charset)
#endif

#ifndef LI_SYMBOL_computed
#define LI_SYMBOL_computed
    LI_SYMBOL(computed)
#endif

#ifndef LI_SYMBOL_connections
#define LI_SYMBOL_connections
    LI_SYMBOL(connections)
#endif

#ifndef LI_SYMBOL_database
#define LI_SYMBOL_database
    LI_SYMBOL(database)
#endif

#ifndef LI_SYMBOL_host
#define LI_SYMBOL_host
    LI_SYMBOL(host)
#endif

#ifndef LI_SYMBOL_id
#define LI_SYMBOL_id
    LI_SYMBOL(id)
#endif

#ifndef LI_SYMBOL_max_async_connections_per_thread
#define LI_SYMBOL_max_async_connections_per_thread
    LI_SYMBOL(max_async_connections_per_thread)
#endif

#ifndef LI_SYMBOL_max_connections
#define LI_SYMBOL_max_connections
    LI_SYMBOL(max_connections)
#endif

#ifndef LI_SYMBOL_max_sync_connections
#define LI_SYMBOL_max_sync_connections
    LI_SYMBOL(max_sync_connections)
#endif

#ifndef LI_SYMBOL_n_connections
#define LI_SYMBOL_n_connections
    LI_SYMBOL(n_connections)
#endif

#ifndef LI_SYMBOL_password
#define LI_SYMBOL_password
    LI_SYMBOL(password)
#endif

#ifndef LI_SYMBOL_port
#define LI_SYMBOL_port
    LI_SYMBOL(port)
#endif

#ifndef LI_SYMBOL_primary_key
#define LI_SYMBOL_primary_key
    LI_SYMBOL(primary_key)
#endif

#ifndef LI_SYMBOL_read_access
#define LI_SYMBOL_read_access
    LI_SYMBOL(read_access)
#endif

#ifndef LI_SYMBOL_read_only
#define LI_SYMBOL_read_only
    LI_SYMBOL(read_only)
#endif

#ifndef LI_SYMBOL_synchronous
#define LI_SYMBOL_synchronous
    LI_SYMBOL(synchronous)
#endif

#ifndef LI_SYMBOL_user
#define LI_SYMBOL_user
    LI_SYMBOL(user)
#endif

#ifndef LI_SYMBOL_validate
#define LI_SYMBOL_validate
    LI_SYMBOL(validate)
#endif

#ifndef LI_SYMBOL_waiting_list
#define LI_SYMBOL_waiting_list
    LI_SYMBOL(waiting_list)
#endif

#ifndef LI_SYMBOL_write_access
#define LI_SYMBOL_write_access
    LI_SYMBOL(write_access)
#endif


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SYMBOLS_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_TYPE_HASHMAP_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_TYPE_HASHMAP_HH


namespace li {

template <typename V>
struct type_hashmap {

  template <typename E, typename F> E& get_cache_entry(int& hash, F)
  {
    // Init hash if needed.
    if (hash == -1)
    {
      std::lock_guard lock(mutex_);
      if (hash == -1)
        hash = counter_++;
    }
    // Init cache if miss.
    if (hash >= values_.size() or !values_[hash].has_value())
    {
      if (values_.size() < hash + 1)
        values_.resize(hash+1);
      values_[hash] = E();
    }

    // Return existing cache entry.
    return std::any_cast<E&>(values_[hash]);
  }
  template <typename K, typename F> V& operator()(F f, K key)
  {
    static int hash = -1;
    return this->template get_cache_entry<std::unordered_map<K, V>>(hash, f)[key];
  }

  template <typename F> V& operator()(F f)
  {
    static int hash = -1;
    return this->template get_cache_entry<V>(hash, f);
  }

private:
  static std::mutex mutex_;
  static int counter_;
  std::vector<std::any> values_;
};

template <typename V>
std::mutex type_hashmap<V>::mutex_;
template <typename V>
int type_hashmap<V>::counter_ = 0;

}

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_TYPE_HASHMAP_HH


namespace li {

/**
 * @brief Provide access to the result of a sql query.
 */
template <typename I> struct sql_result {

  I impl_;

  sql_result() = delete;
  sql_result& operator=(sql_result&) = delete;
  sql_result(const sql_result&) = delete;

  inline ~sql_result() { this->flush_results(); }

  inline void flush_results() { impl_.flush_results(); }

  /**
   * @brief Return the last id generated by a insert comment.
   * With postgresql, it requires the previous command to use the "INSERT [...] returning id;"
   * syntax.
   *
   * @return the last inserted id.
   */
  long long int last_insert_id() { return impl_.last_insert_id(); }

  /**
   * @brief read one row of the result set and advance to next row.
   * Throw an exception if the end of the result set is reached.
   *
   * @return If only one template argument is provided return this same type.
   *         otherwise return a std::tuple of the requested types.
   */
  template <typename T1, typename... T> auto read();
  /**
   * @brief Like read<T>() but do not throw is the eand of the result set is reached, instead
   * it wraps the result in a std::optional that is empty if no data could be fetch.
   *
   */
  template <typename T1, typename... T> auto read_optional();

  /**
   * @brief read one row of the result set and advance to next row.
   * Throw an exception if the end of the result set is reached or if another error happened.
   *
   * Valid calls are:
   *    read(std::tuple<...>& )
   *        fill the tuple according to the current row. The tuple must match
   *        the number of fields in the request.
   *    read(li::metamap<...>& )
   *        fill the metamap according to the current row. The metamap (value types and keys) must
   * match the fields (types and names) of the request. 
   *    read(A& a, B& b, C& c, ...) 
   *        fill a, b, c,...
   *        with each field of the current row. Types of a, b, c... must match the types of the fields.
   *        supported types : only values (i.e not tuples or metamaps) like std::string, integer and floating numbers.
   * @return T the result value.
   */
  template <typename T1, typename... T> bool read(T1&& t1, T&... tail);
  template <typename T> void read(std::optional<T>& o);

  /**
   * @brief Call \param f on each row of the set.
   * The function must take as argument all the select fields of the request, it should
   * follow one of the signature of read (check read documentation for more info).
   * \param f can take arguments by reference to avoid copies but keep in mind that
   * there references will be invalid at the end of the function scope.
   *
   * @example connection.prepare("Select id,post from post_items;")().map(
   *        [&](std::string id, std::string post) {
   *             std::cout << id << post << std::endl; });
   *
   * @param f the function.
   */
  template <typename F> void map(F f);
};

} // namespace li

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_RESULT_HPP
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_RESULT_HPP

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_INTERNAL_UTILS_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_INTERNAL_UTILS_HH


namespace li {
template <typename T> struct is_tuple_after_decay : std::false_type {};
template <typename... T> struct is_tuple_after_decay<std::tuple<T...>> : std::true_type {};

template <typename T> struct is_tuple : is_tuple_after_decay<std::decay_t<T>> {};
template <typename T> struct unconstref_tuple_elements {};
template <typename... T> struct unconstref_tuple_elements<std::tuple<T...>> {
  typedef std::tuple<std::remove_const_t<std::remove_reference_t<T>>...> ret;
};

} // namespace li
#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_INTERNAL_UTILS_HH


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
  typedef std::tuple_element_t<0, TP> TP0;

  auto t = [] {
    static_assert(std::tuple_size_v<TP> > 0, "sql_result map function must take at least 1 argument.");

    if constexpr (is_tuple<TP0>::value || is_metamap<TP0>::value)
      return TP0{};
    else
      return TP{};
  }();

  while (this->read(t)) {
    if constexpr (is_tuple<TP0>::value || is_metamap<TP0>::value)
      map_function(t);
    else
      std::apply(map_function, t);
  }

}
} // namespace li
#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_RESULT_HPP


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_RESULT_HH


namespace li {

struct sqlite_tag {};

template <typename T>
using tuple_remove_references_and_const_t = typename tuple_remove_references_and_const<T>::type;

inline void free_sqlite3_statement(void* s) { sqlite3_finalize((sqlite3_stmt*)s); }

struct sqlite_statement_result {
  sqlite3* db_;
  sqlite3_stmt* stmt_;
  int last_step_ret_;

  inline void flush_results() { sqlite3_reset(stmt_); }

  // Read a tuple or a metamap.
  template <typename T> bool read(T&& output) {

    // Throw is nothing to read.
    if (last_step_ret_ != SQLITE_ROW)
      return false;

    // Tuple
    if constexpr (is_tuple<T>::value) {
      int ncols = sqlite3_column_count(stmt_);
      std::size_t tuple_size = std::tuple_size_v<std::decay_t<T>>;
      if (ncols != tuple_size) {
        std::ostringstream ss;
        ss << "Invalid number of parameters: SQL request has " << ncols
           << " fields but the function to process it has " << tuple_size << " parameters.";
        throw std::runtime_error(ss.str());
      }
      int i = 0;
      auto read_elt = [&](auto& v) {
        this->read_column(i, v, sqlite3_column_type(stmt_, i));
        i++;
      };
      ::li::tuple_map(std::forward<T>(output), read_elt);
    } else // Metamap
    {
      int ncols = sqlite3_column_count(stmt_);
      int filled[metamap_size<T>()];
      for (unsigned i = 0; i < metamap_size<T>(); i++)
        filled[i] = 0;

      for (int i = 0; i < ncols; i++) {
        const char* cname = sqlite3_column_name(stmt_, i);
        bool found = false;
        int j = 0;
        li::map(output, [&](auto k, auto& v) {
          if (!found and !filled[j] and !strcmp(cname, symbol_string(k))) {
            this->read_column(i, v, sqlite3_column_type(stmt_, i));
            filled[j] = 1;
          }
          j++;
        });
      }
    }

    // Go to the next row.
    last_step_ret_ = sqlite3_step(stmt_);
    if (last_step_ret_ != SQLITE_ROW and last_step_ret_ != SQLITE_DONE)
      throw std::runtime_error(sqlite3_errstr(last_step_ret_));

    return true;
  }

  inline long long int last_insert_id() { return sqlite3_last_insert_rowid(db_); }

  inline void read_column(int pos, int& v, int sqltype) {
    if (sqltype != SQLITE_INTEGER)
      throw std::runtime_error(
          "Type mismatch between request result data type and destination type (integer).");
    v = sqlite3_column_int(stmt_, pos);
  }

  inline void read_column(int pos, float& v, int sqltype) {
    if (sqltype != SQLITE_FLOAT)
      throw std::runtime_error(
          "Type mismatch between request result data type and destination type (float).");
    v = float(sqlite3_column_double(stmt_, pos));
  }

  inline void read_column(int pos, double& v, int sqltype) {
    if (sqltype != SQLITE_FLOAT)
      throw std::runtime_error(
          "Type mismatch between request result data type and destination type (double).");
    v = sqlite3_column_double(stmt_, pos);
  }

  inline void read_column(int pos, int64_t& v, int sqltype) {
    if (sqltype != SQLITE_INTEGER)
      throw std::runtime_error(
          "Type mismatch between request result data type and destination type (int64).");
    v = sqlite3_column_int64(stmt_, pos);
  }

  inline void read_column(int pos, std::string& v, int sqltype) {
    if (sqltype != SQLITE_TEXT && sqltype != SQLITE_BLOB)
      throw std::runtime_error(
          "Type mismatch between request result data type and destination type (std::string).");
    auto str = sqlite3_column_text(stmt_, pos);
    auto n = sqlite3_column_bytes(stmt_, pos);
    v = std::move(std::string((const char*)str, n));
  }

  // Todo: Date types
  // template <typename C, typename D>
  // void read_column(int pos, std::chrono::time_point<C, D>& v)
  // {
  //   v = std::chrono::time_point<C, D>(sqlite3_column_int(stmt_, pos));
  // }
};

struct sqlite_statement {
  typedef std::shared_ptr<sqlite3_stmt> stmt_sptr;

  sqlite3* db_;
  sqlite3_stmt* stmt_;
  stmt_sptr stmt_sptr_;

  inline sqlite_statement() {}

  inline sqlite_statement(sqlite3* db, sqlite3_stmt* s)
      : db_(db), stmt_(s), stmt_sptr_(stmt_sptr(s, free_sqlite3_statement)) {}

  // Bind arguments to the request unknowns (marked with ?)
  template <typename... T> sql_result<sqlite_statement_result> operator()(T&&... args) {
    sqlite3_reset(stmt_);
    sqlite3_clear_bindings(stmt_);
    int i = 1;
    li::tuple_map(std::forward_as_tuple(args...), [&](auto& m) {
      int err;
      if ((err = this->bind(stmt_, i, m)) != SQLITE_OK)
        throw std::runtime_error(std::string("Sqlite error during binding: ") +
                                 sqlite3_errmsg(db_));
      i++;
    });

    int last_step_ret = sqlite3_step(stmt_);
    if (last_step_ret != SQLITE_ROW and last_step_ret != SQLITE_DONE)
      throw std::runtime_error(sqlite3_errstr(last_step_ret));

    return sql_result<sqlite_statement_result>{
        sqlite_statement_result{this->db_, this->stmt_, last_step_ret}};
  }

  inline int bind(sqlite3_stmt* stmt, int pos, double d) const {
    return sqlite3_bind_double(stmt, pos, d);
  }

  inline int bind(sqlite3_stmt* stmt, int pos, int d) const { return sqlite3_bind_int(stmt, pos, d); }
  inline int bind(sqlite3_stmt* stmt, int pos, long int d) const {
    return sqlite3_bind_int64(stmt, pos, d);
  }
  inline int bind(sqlite3_stmt* stmt, int pos, long long int d) const {
    return sqlite3_bind_int64(stmt, pos, d);
  }
  inline void bind(sqlite3_stmt* stmt, int pos, sql_null_t) { sqlite3_bind_null(stmt, pos); }
  inline int bind(sqlite3_stmt* stmt, int pos, const char* s) const {
    return sqlite3_bind_text(stmt, pos, s, strlen(s), nullptr);
  }
  inline int bind(sqlite3_stmt* stmt, int pos, const std::string& s) const {
    return sqlite3_bind_text(stmt, pos, s.data(), s.size(), nullptr);
  }
  inline int bind(sqlite3_stmt* stmt, int pos, const std::string_view& s) const {
    return sqlite3_bind_text(stmt, pos, s.data(), s.size(), nullptr);
  }
  inline int bind(sqlite3_stmt* stmt, int pos, const sql_blob& b) const {
    return sqlite3_bind_blob(stmt, pos, b.data(), b.size(), nullptr);
  }
};

inline void free_sqlite3_db(void* db) { sqlite3_close_v2((sqlite3*)db); }

struct sqlite_connection {
  typedef sqlite_tag db_tag;

  typedef std::shared_ptr<sqlite3> db_sptr;
  typedef std::unordered_map<std::string, sqlite_statement> stmt_map;
  typedef std::shared_ptr<std::unordered_map<std::string, sqlite_statement>> stmt_map_ptr;
  typedef std::shared_ptr<std::mutex> mutex_ptr;

  mutex_ptr cache_mutex_;
  sqlite3* db_;
  db_sptr db_sptr_;
  stmt_map_ptr stm_cache_;
  type_hashmap<sqlite_statement> statements_hashmap;

  inline sqlite_connection()
      : db_(nullptr), stm_cache_(new stmt_map()), cache_mutex_(new std::mutex()) // FIXME
  {}

  inline void connect(const std::string& filename,
               int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE) {
    int r = sqlite3_open_v2(filename.c_str(), &db_, flags, nullptr);
    if (r != SQLITE_OK)
      throw std::runtime_error(std::string("Cannot open database ") + filename + " " +
                               sqlite3_errstr(r));

    db_sptr_ = db_sptr(db_, free_sqlite3_db);
  }

  template <typename E> inline void format_error(E&) const {}

  template <typename E, typename T1, typename... T>
  inline void format_error(E& err, T1 a, T... args) const {
    err << a;
    format_error(err, args...);
  }

  template <typename F> sqlite_statement cached_statement(F f) {
    if (statements_hashmap(f).stmt_sptr_.get() == nullptr)
      return prepare(f());
    else
      return statements_hashmap(f);
  }

  inline sqlite_statement prepare(const std::string& req) {
    // std::cout << req << std::endl;
    auto it = stm_cache_->find(req);
    if (it != stm_cache_->end())
      return it->second;

    sqlite3_stmt* stmt;

    int err = sqlite3_prepare_v2(db_, req.c_str(), req.size(), &stmt, nullptr);
    if (err != SQLITE_OK)
      throw std::runtime_error(std::string("Sqlite error during prepare: ") + sqlite3_errmsg(db_) +
                               " statement was: " + req);

    cache_mutex_->lock();
    auto it2 = stm_cache_->insert(it, std::make_pair(req, sqlite_statement(db_, stmt)));
    cache_mutex_->unlock();
    return it2->second;
  }
  inline sql_result<sqlite_statement_result> operator()(const std::string& req) { return prepare(req)(); }

  template <typename T>
  inline std::string type_to_string(const T&, std::enable_if_t<std::is_integral<T>::value>* = 0) {
    return "INTEGER";
  }
  template <typename T>
  inline std::string type_to_string(const T&,
                                    std::enable_if_t<std::is_floating_point<T>::value>* = 0) {
    return "REAL";
  }
  inline std::string type_to_string(const std::string&) { return "TEXT"; }
  inline std::string type_to_string(const sql_blob&) { return "BLOB"; }
  template <unsigned SIZE> inline std::string type_to_string(const sql_varchar<SIZE>&) {
    std::ostringstream ss;
    ss << "VARCHAR(" << SIZE << ')';
    return ss.str();
  }
};

struct sqlite_database {
  typedef sqlite_tag db_tag;

  typedef sqlite_connection connection_type;

  inline sqlite_database() {}

  template <typename... O> sqlite_database(const std::string& path, O... options_) {
    auto options = mmm(options_...);

    path_ = path;
    con_.connect(path, SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    if (has_key(options, s::synchronous)) {
      std::ostringstream ss;
      ss << "PRAGMA synchronous=" << li::get_or(options, s::synchronous, 2);
      con_(ss.str());
    }
  }

  template <typename Y> inline sqlite_connection connect(Y& y) { return con_; }
  inline sqlite_connection connect() { return con_; }

  sqlite_connection con_;
  std::string path_;
};

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQLITE_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_ORM_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_ORM_HH



namespace li {

struct sqlite_tag;
struct mysql_tag;
struct pgsql_tag;

using s::auto_increment;
using s::primary_key;
using s::read_only;

template <typename SCHEMA, typename C> struct sql_orm {

  typedef decltype(std::declval<SCHEMA>().all_fields()) O;
  typedef O object_type;

  ~sql_orm() {

    //assert(0);
  }
  sql_orm(sql_orm&&) = default;
  sql_orm(const sql_orm&) = delete;
  sql_orm& operator=(const sql_orm&) = delete;

  sql_orm(SCHEMA& schema, C&& con) : schema_(schema), con_(std::forward<C>(con)) {}

  template <typename S, typename... A> void call_callback(S s, A&&... args) {
    if constexpr (has_key<decltype(schema_.get_callbacks())>(S{}))
      return schema_.get_callbacks()[s](args...);
  }

  inline auto& drop_table_if_exists() {
    con_(std::string("DROP TABLE IF EXISTS ") + schema_.table_name()).flush_results();
    return *this;
  }

  inline auto& create_table_if_not_exists() {
    std::ostringstream ss;
    ss << "CREATE TABLE if not exists " << schema_.table_name() << " (";

    bool first = true;
    li::tuple_map(schema_.all_info(), [&](auto f) {
      auto f2 = schema_.get_field(f);
      typedef decltype(f) F;
      typedef decltype(f2) F2;
      typedef typename F2::left_t K;
      typedef typename F2::right_t V;

      bool auto_increment = SCHEMA::template is_auto_increment<F>::value;
      bool primary_key = SCHEMA::template is_primary_key<F>::value;
      K k{};
      V v{};

      if (!first)
        ss << ", ";
      ss << li::symbol_string(k) << " ";
      
      if (!std::is_same<typename C::db_tag, pgsql_tag>::value or !auto_increment)
        ss << con_.type_to_string(v);

      if (std::is_same<typename C::db_tag, sqlite_tag>::value) {
        if (auto_increment || primary_key)
          ss << " PRIMARY KEY ";
      }

      if (std::is_same<typename C::db_tag, mysql_tag>::value) {
        if (auto_increment)
          ss << " AUTO_INCREMENT NOT NULL";
        if (primary_key)
          ss << " PRIMARY KEY ";
      }

      if (std::is_same<typename C::db_tag, pgsql_tag>::value) {
        if (auto_increment)
          ss << " SERIAL PRIMARY KEY ";
      }

      first = false;
    });
    ss << ");";
    try {
      con_(ss.str()).flush_results();
    } catch (std::runtime_error e) {
      std::cerr << "Warning: Lithium::sql could not create the " << schema_.table_name() << " sql table."
                << std::endl
                << "You can ignore this message if the table already exists."
                << "The sql error is: " << e.what() << std::endl;
    }
    return *this;
  }

  std::string placeholder_string() {

    if (std::is_same<typename C::db_tag, pgsql_tag>::value) {
      placeholder_pos_++;
      std::stringstream ss;
      ss << '$' << placeholder_pos_;
      return ss.str();
    }
    else return "?";
  }

  template <typename W> void where_clause(W&& cond, std::ostringstream& ss) {
    ss << " WHERE ";
    bool first = true;
    map(cond, [&](auto k, auto v) {
      if (!first)
        ss << " and ";
      first = false;
      ss << li::symbol_string(k) << " = " << placeholder_string();
    });
    ss << " ";
  }

  template <typename... W, typename... A> auto find_one(metamap<W...> where, A&&... cb_args) {

    auto stmt = con_.cached_statement([&] (){ 
        std::ostringstream ss;
        placeholder_pos_ = 0;
        ss << "SELECT ";
        bool first = true;
        O o;
        li::map(o, [&](auto k, auto v) {
          if (!first)
            ss << ",";
          first = false;
          ss << li::symbol_string(k);
        });

        ss << " FROM " << schema_.table_name();
        where_clause(where, ss);
        ss << "LIMIT 1";
        return ss.str();
    });

    O result;
    bool read_success = li::tuple_reduce(metamap_values(where), stmt).template read(metamap_values(result));
    if (read_success)
    {
      call_callback(s::read_access, result, cb_args...);
      return std::make_optional<O>(std::move(result));
    }
    else {
      return std::optional<O>{};
    }
  }

  template <typename A, typename B, typename... O, typename... W>
  auto find_one(metamap<O...>&& o, assign_exp<A, B>&& w1, W... ws) {
    return find_one(cat(o, mmm(w1)), std::forward<W>(ws)...);
  }
  template <typename A, typename B, typename... W> auto find_one(assign_exp<A, B>&& w1, W&&... ws) {
    return find_one(mmm(w1), std::forward<W>(ws)...);
  }

  template <typename W> bool exists(W&& cond) {

    O o;
    auto stmt = con_.cached_statement([&] { 
        std::ostringstream ss;
        placeholder_pos_ = 0;
        ss << "SELECT count(*) FROM " << schema_.table_name();
        where_clause(cond, ss);
        ss << "LIMIT 1";
        return ss.str();
    });

    return li::tuple_reduce(metamap_values(cond), stmt).template read<int>();
  }

  template <typename A, typename B, typename... W> auto exists(assign_exp<A, B> w1, W... ws) {
    return exists(mmm(w1, ws...));
  }
  // Save a ll fields except auto increment.
  // The db will automatically fill auto increment keys.
  template <typename N, typename... A> auto insert(N&& o, A&&... cb_args) {

    auto values = schema_.without_auto_increment();
    map(o, [&](auto k, auto& v) { values[k] = o[k]; });

    call_callback(s::validate, values, cb_args...);
    call_callback(s::before_insert, values, cb_args...);


    auto stmt = con_.cached_statement([&] { 
        std::ostringstream ss;
        std::ostringstream vs;

        placeholder_pos_ = 0;
        ss << "INSERT into " << schema_.table_name() << "(";

        bool first = true;
        li::map(values, [&](auto k, auto v) {
          if (!first) {
            ss << ",";
            vs << ",";
          }
          first = false;
          ss << li::symbol_string(k);
          vs << placeholder_string();
        });

        ss << ") VALUES (" << vs.str() << ")";

        if (std::is_same<typename C::db_tag, pgsql_tag>::value &&
            has_key(schema_.all_fields(), s::id))
          ss << " returning id;";
        return ss.str();
    });

    auto request_res = li::reduce(values, stmt);

    call_callback(s::after_insert, o, cb_args...);

    if constexpr(has_key<decltype(schema_.all_fields())>(s::id))
      return request_res.last_insert_id();
    else return request_res.flush_results();
  };

  template <typename A, typename B, typename... O, typename... W>
  auto insert(metamap<O...>&& o, assign_exp<A, B> w1, W... ws) {
    return insert(cat(o, mmm(w1)), ws...);
  }
  template <typename A, typename B, typename... W>
  auto insert(assign_exp<A, B> w1, W... ws) {
    return insert(mmm(w1), ws...);
  }

  // template <typename S, typename V, typename... A>
  // long long int insert(const assign_exp<S, V>& a, A&&... tail) {
  //   auto m = mmm(a, tail...);
  //   return insert(m);
  // }

  // Iterate on all the rows of the table.
  template <typename F> void forall(F f) {

    typedef decltype(schema_.all_fields()) O;

    auto stmt = con_.cached_statement([&] { 
        std::ostringstream ss;
        placeholder_pos_ = 0;
      
        ss << "SELECT ";
        bool first = true;
        O o;
        li::map(o, [&](auto k, auto v) {
          if (!first)
            ss << ",";
          first = false;
          ss << li::symbol_string(k);
        });
      
        ss << " FROM " << schema_.table_name();
        return ss.str();
    });
    // stmt().map([&](const O& o) { f(o); });
    using values_tuple = tuple_remove_references_and_const_t<decltype(metamap_values(std::declval<O>()))>;
    using keys_tuple = decltype(metamap_keys(std::declval<O>()));
    stmt().map([&](const values_tuple& values) { f(forward_tuple_as_metamap(keys_tuple{}, values)); });

  }

  // Update N's members except auto increment members.
  // N must have at least one primary key named id.
  // Only postgres is supported for now.
  template <typename N, typename... CB> void bulk_update(const N& elements, CB&&... args) {

    if constexpr(!std::is_same<typename C::db_tag, pgsql_tag>::value)
      for (const auto& o : elements)
        this->update(o);
    else
    {
      
      auto stmt = con_.cached_statement([&] { 
        std::ostringstream ss;
        ss << "UPDATE " << schema_.table_name() << " SET ";

        int nfields = metamap_size<decltype(elements[0])>();
        bool first = true;
        map(elements[0], [&](auto k, auto v) {
          if (!first)
            ss << ",";
          if (not li::has_key(schema_.primary_key(), k))
          {
            ss << li::symbol_string(k) << " = tmp." << li::symbol_string(k);
            first = false;
          }
        });

        ss << " FROM (VALUES ";
        for (int i = 0; i < elements.size(); i++)
        {
          if (i != 0) ss << ',';
          ss << "(";
          for (int j = 0; j < nfields; j++)
          {
            if (j != 0) ss << ",";
            ss << "$" <<  1+i*nfields+j << "::int";
          }
          ss << ")";
        }
      
        ss << ") AS tmp(";
        first = true;
        map(elements[0], [&](auto k, auto v) {
          if (!first)
            ss << ",";
          first = false;
          ss << li::symbol_string(k);
        });
        ss << ") WHERE tmp.id = " << schema_.table_name() << ".id";      
        // std::cout << ss.str() << std::endl;
        return ss.str();
      }, elements.size());

      for (const auto& o : elements)
      {
        call_callback(s::validate, o, args...);
        call_callback(s::write_access, o, args...);
        call_callback(s::before_update, o, args...);
      }

      stmt(elements).flush_results();
    }
  }

  // Update N's members except auto increment members.
  // N must have at least one primary key.
  template <typename N, typename... CB> void update(const N& o, CB&&... args) {
    // check if N has at least one member of PKS.

    call_callback(s::validate, o, args...);
    call_callback(s::write_access, o, args...);
    call_callback(s::before_update, o, args...);

    // static_assert(metamap_size<decltype(intersect(o, schema_.read_only()))>(),
    //"You cannot give read only fields to the orm update method.");

    auto to_update = substract(o, schema_.read_only());
    auto pk = intersection(o, schema_.primary_key());

    auto stmt = con_.cached_statement([&] { 
        static_assert(metamap_size<decltype(pk)>() > 0,
                      "You must provide at least one primary key to update an object.");
        std::ostringstream ss;
        placeholder_pos_ = 0;
        ss << "UPDATE " << schema_.table_name() << " SET ";

        bool first = true;

        map(to_update, [&](auto k, auto v) {
          if (!first)
            ss << ",";
          first = false;
          ss << li::symbol_string(k) << " = " << placeholder_string();
        });

        where_clause(pk, ss);
        return ss.str();
    });

    li::tuple_reduce(std::tuple_cat(metamap_values(to_update), metamap_values(pk)), stmt);

    call_callback(s::after_update, o, args...);
  }

  template <typename A, typename B, typename... O, typename... W>
  void update(metamap<O...>&& o, assign_exp<A, B> w1, W... ws) {
    return update(cat(o, mmm(w1)), ws...);
  }
  template <typename A, typename B, typename... W> void update(assign_exp<A, B> w1, W... ws) {
    return update(mmm(w1), ws...);
  }

  inline int count() {
    return con_.prepare(std::string("SELECT count(*) from ") + schema_.table_name())().template read<int>();
  }

  template <typename N, typename... CB> void remove(const N& o, CB&&... args) {

    call_callback(s::before_remove, o, args...);

    auto stmt = con_.cached_statement([&] { 
      std::ostringstream ss;
      placeholder_pos_ = 0;
      ss << "DELETE from " << schema_.table_name() << " WHERE ";

      bool first = true;
      map(schema_.primary_key(), [&](auto k, auto v) {
        if (!first)
          ss << " and ";
        first = false;
        ss << li::symbol_string(k) << " = " << placeholder_string();
      });
      return ss.str();
    });

    auto pks = intersection(o, schema_.primary_key());
    li::reduce(pks, stmt);

    call_callback(s::after_remove, o, args...);
  }
  template <typename A, typename B, typename... O, typename... W>
  void remove(metamap<O...>&& o, assign_exp<A, B> w1, W... ws) {
    return remove(cat(o, mmm(w1)), ws...);
  }
  template <typename A, typename B, typename... W> void remove(assign_exp<A, B> w1, W... ws) {
    return remove(mmm(w1), ws...);
  }

  auto& schema() { return schema_; }

  C& backend_connection() { return con_; }

  SCHEMA schema_;
  C con_;
  int placeholder_pos_ = 0;
};

template <typename... F> struct orm_fields {

  orm_fields(F... fields) : fields_(fields...) {
    static_assert(sizeof...(F) == 0 || metamap_size<decltype(this->primary_key())>() != 0,
                  "You must give at least one primary key to the ORM. Use "
                  "s::your_field_name(s::primary_key) to add a primary_key");
  }

  // Field extractor.
  template <typename M> auto get_field(M m) { return m; }
  template <typename M, typename T> auto get_field(assign_exp<M, T> e) { return e; }
  template <typename M, typename T, typename... A>
  auto get_field(assign_exp<function_call_exp<M, A...>, T> e) {
    return assign_exp<M, T>{M{}, e.right};
  }

  // template <typename M> struct get_field { typedef M ret; };
  // template <typename M, typename T> struct get_field<assign_exp<M, T>> {
  //   typedef assign_exp<M, T> ret;
  //   static auto ctor() { return assign_exp<M, T>{M{}, T()}; }
  // };
  // template <typename M, typename T, typename... A>
  // struct get_field<assign_exp<function_call_exp<M, A...>, T>> : public get_field<assign_exp<M,
  // T>> {
  // };

// get_field<E>::ctor();
// field attributes checks.
#define CHECK_FIELD_ATTR(ATTR)                                                                     \
  template <typename M> struct is_##ATTR : std::false_type {};                                     \
  template <typename M, typename T, typename... A>                                                 \
  struct is_##ATTR<assign_exp<function_call_exp<M, A...>, T>>                                      \
      : std::disjunction<std::is_same<std::decay_t<A>, s::ATTR##_t>...> {};                        \
                                                                                                   \
  auto ATTR() {                                                                                    \
    return tuple_map_reduce(fields_,                                                               \
                            [this](auto e) {                                                       \
                              typedef std::remove_reference_t<decltype(e)> E;                      \
                              if constexpr (is_##ATTR<E>::value)                                   \
                                return get_field(e);                                               \
                              else                                                                 \
                                return skip{};                                                     \
                            },                                                                     \
                            make_metamap_skip);                                                    \
  }

  CHECK_FIELD_ATTR(primary_key);
  CHECK_FIELD_ATTR(read_only);
  CHECK_FIELD_ATTR(auto_increment);
  CHECK_FIELD_ATTR(computed);
#undef CHECK_FIELD_ATTR

  // Do not remove this comment, this is used by the symbol generation.
  // s::primary_key s::read_only s::auto_increment s::computed

  auto all_info() { return fields_; }

  auto all_fields() {

    return tuple_map_reduce(fields_,
                            [this](auto e) {
                              // typedef std::remove_reference_t<decltype(e)> E;
                              return get_field(e);
                            },
                            [](auto... e) { return mmm(e...); });
  }

  auto without_auto_increment() { return substract(all_fields(), auto_increment()); }
  auto all_fields_except_computed() {
    return substract(substract(all_fields(), computed()), auto_increment());
  }

  std::tuple<F...> fields_;
};

template <typename DB, typename MD = orm_fields<>, typename CB = decltype(mmm())>
struct sql_orm_schema : public MD {

  sql_orm_schema(DB& db, const std::string& table_name, CB cb = CB(), MD md = MD())
      : MD(md), database_(db), table_name_(table_name), callbacks_(cb) {}

  inline auto connect() { return sql_orm{*this, database_.connect()}; }
  template <typename Y>
  inline auto connect(Y& y) { return sql_orm{*this, database_.connect(y)}; }

  const std::string& table_name() const { return table_name_; }
  auto get_callbacks() const { return callbacks_; }

  template <typename... P> auto callbacks(P... params_list) const {
    auto cbs = mmm(params_list...);
    auto allowed_callbacks = mmm(s::before_insert, s::before_remove, s::before_update,
                                 s::after_insert, s::after_remove, s::after_update, s::validate);

    static_assert(
        metamap_size<decltype(substract(cbs, allowed_callbacks))>() == 0,
        "The only supported callbacks are: s::before_insert, s::before_remove, s::before_update,"
        " s::after_insert, s::after_remove, s::after_update, s::validate");
    return sql_orm_schema<DB, MD, decltype(cbs)>(database_, table_name_, cbs,
                                                 *static_cast<const MD*>(this));
  }

  template <typename... P> auto fields(P... p) const {
    return sql_orm_schema<DB, orm_fields<P...>, CB>(database_, table_name_, callbacks_,
                                                    orm_fields<P...>(p...));
  }

  DB& database_;
  std::string table_name_;
  CB callbacks_;
};

}; // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_ORM_HH

