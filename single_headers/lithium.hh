// Author: Matthieu Garrigues matthieu.garrigues@gmail.com
//
// Single header version the lithium library.
// https://github.com/matt-42/lithium
//
// This file is generated do not edit it.

#pragma once

#include <cstring>
#include <unistd.h>
#include <unordered_map>
#include <string>
#include <sqlite3.h>
#include <string.h>
#include <stdio.h>
#include <optional>
#include <fcntl.h>
#include <sys/mman.h>
#include <vector>
#include <functional>
#include <memory>
#include <boost/lexical_cast.hpp>
#include <sys/types.h>
#include <cassert>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <chrono>
#include <thread>
#include <netinet/tcp.h>
#include <sys/stat.h>
#include <variant>
#include <any>
#include <boost/context/continuation.hpp>
#include <set>
#include <string_view>
#include <sys/uio.h>
#include <atomic>
#include <tuple>
#include <stdlib.h>
#include <netdb.h>
#include <mysql.h>
#include <mutex>
#include <sstream>
#include <sys/epoll.h>
#include <random>
#include <signal.h>
#include <errno.h>
#include <deque>
#include <utility>
#include <cmath>
#include <iostream>
#include <map>

#if defined(_MSC_VER)
#include <io.h>
#include <ciso646>
#endif // _MSC_VER


#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQLITE
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQLITE

#if defined(_MSC_VER)
#endif


#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_CALLABLE_TRAITS_CALLABLE_TRAITS
#define LITHIUM_SINGLE_HEADER_GUARD_LI_CALLABLE_TRAITS_CALLABLE_TRAITS

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

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_CALLABLE_TRAITS_CALLABLE_TRAITS

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_METAMAP
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_METAMAP


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

template <typename F, typename... M> decltype(auto) find_first(metamap<M...>&& map, F fun);

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

  inline metamap(typename M1::_iod_value_type&& m1, typename Ms::_iod_value_type&&... members) : M1{m1}, Ms{std::forward<typename Ms::_iod_value_type>(members)}... {}
  inline metamap(M1&& m1, Ms&&... members) : M1(m1), Ms(std::forward<Ms>(members))... {}
  inline metamap(const M1& m1, const Ms&... members) : M1(m1), Ms((members))... {}

  // Assignemnt ?

  // Retrive a value.
  template <typename K> decltype(auto) operator[](K k) { return symbol_member_access(*this, k); }

  template <typename K> decltype(auto) operator[](K k) const {
    return symbol_member_access(*this, k);
  }
};

template <> struct metamap<> {
  typedef metamap<> self;
  // Constructors.
  inline metamap() = default;
  // inline metamap(self&&) = default;
  inline metamap(const self&) = default;
  // self& operator=(const self&) = default;

  // metamap(self& other)
  //  : metamap(const_cast<const self&>(other)) {}

  // Assignemnt ?

  // Retrive a value.
  template <typename K> decltype(auto) operator[](K k) { return symbol_member_access(*this, k); }

  template <typename K> decltype(auto) operator[](K k) const {
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

template <typename... Ks> decltype(auto) metamap_values(const metamap<Ks...>& map) {
  return std::forward_as_tuple(map[typename Ks::_iod_symbol_type()]...);
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

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_CAT
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_CAT

namespace li {

template <typename... T, typename... U>
inline decltype(auto) cat(const metamap<T...>& a, const metamap<U...>& b) {
  return metamap<T..., U...>(*static_cast<const T*>(&a)..., *static_cast<const U*>(&b)...);
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_CAT

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_INTERSECTION
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_INTERSECTION

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_MAKE_METAMAP_SKIP
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_MAKE_METAMAP_SKIP

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_MAKE
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_MAKE

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SYMBOL_AST
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SYMBOL_AST


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

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SYMBOL_AST

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SYMBOL_SYMBOL
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SYMBOL_SYMBOL


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

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SYMBOL_SYMBOL


namespace li {

template <typename... Ms> struct metamap;

namespace internal {

template <typename S, typename V> decltype(auto) exp_to_variable_ref(const assign_exp<S, V>& e) {
  return make_variable_reference(S{}, e.right);
}

template <typename S, typename V> decltype(auto) exp_to_variable(const assign_exp<S, V>& e) {
  typedef std::remove_const_t<std::remove_reference_t<V>> vtype;
  return make_variable(S{}, e.right);
}

template <typename S> decltype(auto) exp_to_variable(const symbol<S>& e) {
  return exp_to_variable(S() = int());
}

template <typename... T> inline decltype(auto) make_metamap_helper(T&&... args) {
  return metamap<T...>(std::forward<T>(args)...);
}

} // namespace internal

// Store copies of values in the map
static struct {
  template <typename... T> inline decltype(auto) operator()(T&&... args) const {
    // Copy values.
    return internal::make_metamap_helper(internal::exp_to_variable(std::forward<T>(args))...);
  }
} mmm;

// Store references of values in the map
template <typename... T> inline decltype(auto) make_metamap_reference(T&&... args) {
  // Keep references.
  return internal::make_metamap_helper(internal::exp_to_variable_ref(std::forward<T>(args))...);
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_MAKE


namespace li {

struct skip {};
static struct {

  template <typename... M, typename... T>
  inline decltype(auto) run(metamap<M...> map, skip, T&&... args) const {
    return run(map, std::forward<T>(args)...);
  }

  template <typename T1, typename... M, typename... T>
  inline decltype(auto) run(metamap<M...> map, T1&& a, T&&... args) const {
    return run(
        cat(map, internal::make_metamap_helper(internal::exp_to_variable(std::forward<T1>(a)))),
        std::forward<T>(args)...);
  }

  template <typename... M> inline decltype(auto) run(metamap<M...> map) const { return map; }

  template <typename... T> inline decltype(auto) operator()(T&&... args) const {
    // Copy values.
    return run(metamap<>{}, std::forward<T>(args)...);
  }

} make_metamap_skip;

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_MAKE_METAMAP_SKIP

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_MAP_REDUCE
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_MAP_REDUCE

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_TUPLE_UTILS
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_TUPLE_UTILS


namespace li {

template <typename... E, typename F> void apply_each(F&& f, E&&... e) {
  (void)std::initializer_list<int>{((void)f(std::forward<E>(e)), 0)...};
}

template <typename... E, typename F, typename R>
auto tuple_map_reduce_impl(F&& f, R&& reduce, E&&... e) {
  return reduce(f(std::forward<E>(e))...);
}

template <typename T, typename F> void tuple_map(T&& t, F&& f) {
  return std::apply([&](auto&&... e) { apply_each(f, std::forward<decltype(e)>(e)...); },
                    std::forward<T>(t));
}

template <typename T, typename F> auto tuple_reduce(T&& t, F&& f) {
  return std::apply(std::forward<F>(f), std::forward<T>(t));
}

template <typename T, typename F, typename R>
decltype(auto) tuple_map_reduce(T&& m, F map, R reduce) {
  auto fun = [&](auto... e) { return tuple_map_reduce_impl(map, reduce, e...); };
  return std::apply(fun, m);
}

template <typename F> inline std::tuple<> tuple_filter_impl() { return std::make_tuple(); }

template <typename F, typename... M, typename M1> auto tuple_filter_impl(M1 m1, M... m) {
  if constexpr (std::is_same<M1, F>::value)
    return tuple_filter_impl<F>(m...);
  else
    return std::tuple_cat(std::make_tuple(m1), tuple_filter_impl<F>(m...));
}

template <typename F, typename... M> auto tuple_filter(const std::tuple<M...>& m) {

  auto fun = [](auto... e) { return tuple_filter_impl<F>(e...); };
  return std::apply(fun, m);
}

} // namespace li
#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_TUPLE_UTILS


namespace li {

// Map a function(key, value) on all kv pair
template <typename... M, typename F> void map(const metamap<M...>& m, F fun) {
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
template <typename... M, typename F> void map(metamap<M...>& m, F fun) {
  auto apply = [&](auto key) -> decltype(auto) { return fun(key, m[key]); };

  apply_each(apply, typename M::_iod_symbol_type{}...);
}

template <typename... E, typename F, typename R> auto apply_each2(F&& f, R&& r, E&&... e) {
  return r(f(std::forward<E>(e))...);
  //(void)std::initializer_list<int>{
  //  ((void)f(std::forward<E>(e)), 0)...};
}

// Map a function(key, value) on all kv pair an reduce
// all the results value with the reduce(r1, r2, ...) function.
template <typename... M, typename F, typename R>
decltype(auto) map_reduce(const metamap<M...>& m, F map, R reduce) {
  auto apply = [&](auto key) -> decltype(auto) {
    // return map(key, std::forward<decltype(m[key])>(m[key]));
    return map(key, m[key]);
  };

  return apply_each2(apply, reduce, typename M::_iod_symbol_type{}...);
  // return reduce(apply(typename M::_iod_symbol_type{})...);
}

// Map a function(key, value) on all kv pair an reduce
// all the results value with the reduce(r1, r2, ...) function.
template <typename... M, typename R> decltype(auto) reduce(const metamap<M...>& m, R reduce) {
  return reduce(m[typename M::_iod_symbol_type{}]...);
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_MAP_REDUCE



namespace li {

template <typename... T, typename... U>
inline decltype(auto) intersection(const metamap<T...>& a, const metamap<U...>& b) {
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

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_INTERSECTION

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_SUBSTRACT
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_SUBSTRACT


namespace li {

template <typename... T, typename... U>
inline auto substract(const metamap<T...>& a, const metamap<U...>& b) {
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

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_SUBSTRACT


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_METAMAP

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_COMMON
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_COMMON


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
#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_COMMON

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SYMBOLS
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SYMBOLS

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

#ifndef LI_SYMBOL_write_access
#define LI_SYMBOL_write_access
    LI_SYMBOL(write_access)
#endif


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SYMBOLS

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_TYPE_HASHMAP
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_TYPE_HASHMAP


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

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_TYPE_HASHMAP


namespace li {

struct sqlite_tag {};

template <typename T> struct tuple_remove_references_and_const;
template <typename... T> struct tuple_remove_references_and_const<std::tuple<T...>> {
  typedef std::tuple<std::remove_const_t<std::remove_reference_t<T>>...> type;
};

template <typename T>
using tuple_remove_references_and_const_t = typename tuple_remove_references_and_const<T>::type;

void free_sqlite3_statement(void* s) { sqlite3_finalize((sqlite3_stmt*)s); }

struct sqlite_statement {
  typedef std::shared_ptr<sqlite3_stmt> stmt_sptr;

  sqlite_statement() {}

  sqlite_statement(sqlite3* db, sqlite3_stmt* s)
      : db_(db), stmt_(s), stmt_sptr_(stmt_sptr(s, free_sqlite3_statement)),
        ready_for_reading_(false) {}

  void wait () {}

  template <typename... A> void row_to_metamap(metamap<A...>& o) {
    int ncols = sqlite3_column_count(stmt_);
    int filled[sizeof...(A)];
    for (unsigned i = 0; i < sizeof...(A); i++)
      filled[i] = 0;

    for (int i = 0; i < ncols; i++) {
      const char* cname = sqlite3_column_name(stmt_, i);
      bool found = false;
      int j = 0;
      li::map(o, [&](auto k, auto& v) {
        if (!found and !filled[j] and !strcmp(cname, symbol_string(k))) {
          this->read_column(i, v);
          filled[j] = 1;
        }
        j++;
      });
    }
  }

  template <typename... A> void row_to_tuple(std::tuple<A...>& o) {
    int ncols = sqlite3_column_count(stmt_);
    if (ncols != sizeof...(A)) {
      std::ostringstream ss;
      ss << "Invalid number of parameters: SQL request has " << ncols
         << " fields but the function to process it has " << sizeof...(A) << " parameters.";
      throw std::runtime_error(ss.str());
    }
    int i = 0;
    auto read_elt = [&](auto& v) {
      this->read_column(i, v);
      i++;
    };
    ::li::tuple_map(o, read_elt);
  }

  // Bind arguments to the request unknowns (marked with ?)
  template <typename... T> auto& operator()(T&&... args) {
    ready_for_reading_ = true;
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

    last_step_ret_ = sqlite3_step(stmt_);
    if (last_step_ret_ != SQLITE_ROW and last_step_ret_ != SQLITE_DONE)
      throw std::runtime_error(sqlite3_errstr(last_step_ret_));

    return *this;
  }

  // Fill a metamap with the current result row.
  template <typename... A> int fetch(metamap<A...>& o) {

    if (not ready_for_reading_)
      this->operator()();
    if (empty())
      return false;
    row_to_metamap(o);
    ready_for_reading_ = false;
    return true;
  }

  // Fill an simple object (int or string) with the current result row.
  template <typename T> int fetch(T& o) {

    if (not ready_for_reading_)
      this->operator()();
    if (empty())
      return false;
    this->read_column(0, o);
    ready_for_reading_ = false;
    return true;
  }

  template <typename T> void read(std::optional<T>& o) {
    T t;
    if (fetch(t))
      o = std::optional<T>(t);
    else
      o = std::optional<T>();
  }

  template <typename T> T read() {
    T t;
    if (!fetch(t))
      throw std::runtime_error("Request did not return any data.");
    return t;
  }

  template <typename T> std::optional<T> read_optional() {
    T t;
    if (fetch(t))
      return std::optional<T>(t);
    else
      return std::optional<T>();
  }

  long long int last_insert_id() { return sqlite3_last_insert_rowid(db_); }

  int empty() { return last_step_ret_ != SQLITE_ROW; }

  // Apply a function to all result rows.
  template <typename F> void map(F f) {
    if (not ready_for_reading_)
      this->operator()();

    while (last_step_ret_ == SQLITE_ROW) {
      typedef callable_arguments_tuple_t<F> tp;
      typedef std::remove_reference_t<std::tuple_element_t<0, tp>> T;
      if constexpr (li::is_metamap<T>::value) {
        T o;
        row_to_metamap(o);
        f(o);
        last_step_ret_ = sqlite3_step(stmt_);
      } else {
        typedef tuple_remove_references_and_const_t<std::remove_pointer_t<tp>> T;
        T o;
        row_to_tuple(o);
        std::apply(f, o);
        last_step_ret_ = sqlite3_step(stmt_);
      }
    }
    ready_for_reading_ = false;
  }

  template <typename V> void append_to(V& v) {
    (*this) | [&v](typename V::value_type& o) { v.push_back(o); };
  }

  void read_column(int pos, int& v) { v = sqlite3_column_int(stmt_, pos); }
  void read_column(int pos, float& v) { v = float(sqlite3_column_double(stmt_, pos)); }
  void read_column(int pos, double& v) { v = sqlite3_column_double(stmt_, pos); }
  void read_column(int pos, int64_t& v) { v = sqlite3_column_int64(stmt_, pos); }
  void read_column(int pos, std::string& v) {
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

  int bind(sqlite3_stmt* stmt, int pos, double d) const {
    return sqlite3_bind_double(stmt, pos, d);
  }

  int bind(sqlite3_stmt* stmt, int pos, int d) const { return sqlite3_bind_int(stmt, pos, d); }
  int bind(sqlite3_stmt* stmt, int pos, long int d) const {
    return sqlite3_bind_int64(stmt, pos, d);
  }
  int bind(sqlite3_stmt* stmt, int pos, long long int d) const {
    return sqlite3_bind_int64(stmt, pos, d);
  }
  void bind(sqlite3_stmt* stmt, int pos, sql_null_t) { sqlite3_bind_null(stmt, pos); }
  int bind(sqlite3_stmt* stmt, int pos, const char* s) const {
    return sqlite3_bind_text(stmt, pos, s, strlen(s), nullptr);
  }
  int bind(sqlite3_stmt* stmt, int pos, const std::string& s) const {
    return sqlite3_bind_text(stmt, pos, s.data(), s.size(), nullptr);
  }
  int bind(sqlite3_stmt* stmt, int pos, const std::string_view& s) const {
    return sqlite3_bind_text(stmt, pos, s.data(), s.size(), nullptr);
  }
  int bind(sqlite3_stmt* stmt, int pos, const sql_blob& b) const {
    return sqlite3_bind_blob(stmt, pos, b.data(), b.size(), nullptr);
  }

  sqlite3* db_;
  sqlite3_stmt* stmt_;
  stmt_sptr stmt_sptr_;
  int last_step_ret_;
  bool ready_for_reading_;
};

void free_sqlite3_db(void* db) { sqlite3_close_v2((sqlite3*)db); }

struct sqlite_connection {
  typedef sqlite_tag db_tag;
  
  typedef std::shared_ptr<sqlite3> db_sptr;
  typedef std::unordered_map<std::string, sqlite_statement> stmt_map;
  typedef std::shared_ptr<std::unordered_map<std::string, sqlite_statement>> stmt_map_ptr;
  typedef std::shared_ptr<std::mutex> mutex_ptr;

  sqlite_connection()
      : db_(nullptr), stm_cache_(new stmt_map()), cache_mutex_(new std::mutex()) // FIXME
  {}

  void connect(const std::string& filename,
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

  template <typename F>
  sqlite_statement cached_statement(F f) {
    if (statements_hashmap(f).stmt_sptr_.get() == nullptr)
      return prepare(f());
    else
      return statements_hashmap(f);
  }

  sqlite_statement prepare(const std::string& req) {
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
  sqlite_statement operator()(const std::string& req) { return prepare(req)(); }

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

  mutex_ptr cache_mutex_;
  sqlite3* db_;
  db_sptr db_sptr_;
  stmt_map_ptr stm_cache_;
  type_hashmap<sqlite_statement> statements_hashmap;
};

struct sqlite_database {
  typedef sqlite_connection connection_type;

  sqlite_database() {}

  template <typename... O> sqlite_database(const std::string& path, O... options_) {
    auto options = mmm(options_...);

    path_ = path;
    con_.connect(path, SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);
    if (has_key(options, s::synchronous)) {
      std::ostringstream ss;
      ss << "PRAGMA synchronous=" << li::get_or(options, s::synchronous, 2);
      con_(ss.str())();
    }
  }

  template <typename Y>
  inline sqlite_connection connect(Y& y) { return con_; }
  inline sqlite_connection connect() { return con_; }

  sqlite_connection con_;
  std::string path_;
};

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQLITE

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_ORM
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_ORM

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_SYMBOLS
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_SYMBOLS

#ifndef LI_SYMBOL_blocking
#define LI_SYMBOL_blocking
    LI_SYMBOL(blocking)
#endif

#ifndef LI_SYMBOL_create_secret_key
#define LI_SYMBOL_create_secret_key
    LI_SYMBOL(create_secret_key)
#endif

#ifndef LI_SYMBOL_date_thread
#define LI_SYMBOL_date_thread
    LI_SYMBOL(date_thread)
#endif

#ifndef LI_SYMBOL_hash_password
#define LI_SYMBOL_hash_password
    LI_SYMBOL(hash_password)
#endif

#ifndef LI_SYMBOL_https_cert
#define LI_SYMBOL_https_cert
    LI_SYMBOL(https_cert)
#endif

#ifndef LI_SYMBOL_https_key
#define LI_SYMBOL_https_key
    LI_SYMBOL(https_key)
#endif

#ifndef LI_SYMBOL_id
#define LI_SYMBOL_id
    LI_SYMBOL(id)
#endif

#ifndef LI_SYMBOL_linux_epoll
#define LI_SYMBOL_linux_epoll
    LI_SYMBOL(linux_epoll)
#endif

#ifndef LI_SYMBOL_name
#define LI_SYMBOL_name
    LI_SYMBOL(name)
#endif

#ifndef LI_SYMBOL_non_blocking
#define LI_SYMBOL_non_blocking
    LI_SYMBOL(non_blocking)
#endif

#ifndef LI_SYMBOL_nthreads
#define LI_SYMBOL_nthreads
    LI_SYMBOL(nthreads)
#endif

#ifndef LI_SYMBOL_one_thread_per_connection
#define LI_SYMBOL_one_thread_per_connection
    LI_SYMBOL(one_thread_per_connection)
#endif

#ifndef LI_SYMBOL_path
#define LI_SYMBOL_path
    LI_SYMBOL(path)
#endif

#ifndef LI_SYMBOL_primary_key
#define LI_SYMBOL_primary_key
    LI_SYMBOL(primary_key)
#endif

#ifndef LI_SYMBOL_read_only
#define LI_SYMBOL_read_only
    LI_SYMBOL(read_only)
#endif

#ifndef LI_SYMBOL_select
#define LI_SYMBOL_select
    LI_SYMBOL(select)
#endif

#ifndef LI_SYMBOL_server_thread
#define LI_SYMBOL_server_thread
    LI_SYMBOL(server_thread)
#endif

#ifndef LI_SYMBOL_session_id
#define LI_SYMBOL_session_id
    LI_SYMBOL(session_id)
#endif

#ifndef LI_SYMBOL_update_secret_key
#define LI_SYMBOL_update_secret_key
    LI_SYMBOL(update_secret_key)
#endif

#ifndef LI_SYMBOL_user_id
#define LI_SYMBOL_user_id
    LI_SYMBOL(user_id)
#endif


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_SYMBOLS



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
    con_(std::string("DROP TABLE IF EXISTS ") + schema_.table_name()).wait();
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
      con_(ss.str()).wait();
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

    auto res = li::tuple_reduce(metamap_values(where), stmt).template read_optional<O>();
    if (res)
      call_callback(s::read_access, *res, cb_args...);
    return res;
  }

  template <typename A, typename B, typename... O, typename... W>
  auto find_one(metamap<O...>&& o, assign_exp<A, B> w1, W... ws) {
    return find_one(cat(o, mmm(w1)), ws...);
  }
  template <typename A, typename B, typename... W> auto find_one(assign_exp<A, B> w1, W... ws) {
    return find_one(mmm(w1), ws...);
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
    else return request_res.wait();
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
        ss << "SELECT * from " << schema_.table_name();
        return ss.str();
    });
    stmt().map([&](const O& o) { f(o); });
  }

  // Update N's members except auto increment members.
  // N must have at least one primary key.
  template <typename N, typename... CB> void bulk_update(const N& elements, CB&&... args) {

    if (!std::is_same<typename C::db_tag, pgsql_tag>::value)
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
        this->update(o);
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

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_ORM

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL


#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_ASYNC_WRAPPER
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_ASYNC_WRAPPER



namespace li {

// Blocking version.
struct mysql_functions_blocking {
  enum { is_blocking = true };

#define LI_MYSQL_BLOCKING_WRAPPER(ERR, FN)                                                              \
  template <typename A1, typename... A> auto FN(std::shared_ptr<int> connection_status, A1 a1, A&&... a) {\
    int ret = ::FN(a1, std::forward<A>(a)...); \
    if (ret and ret != MYSQL_NO_DATA and ret != MYSQL_DATA_TRUNCATED) \
    { \
      *connection_status = 1;\
      throw std::runtime_error(std::string("Mysql error: ") + ERR(a1));\
    } \
    return ret; }

  MYSQL_ROW mysql_fetch_row(std::shared_ptr<int> connection_status, MYSQL_RES* res) { return ::mysql_fetch_row(res); }
  int mysql_free_result(std::shared_ptr<int> connection_status, MYSQL_RES* res) { ::mysql_free_result(res); return 0; }
  //LI_MYSQL_BLOCKING_WRAPPER(mysql_error, mysql_fetch_row)
  LI_MYSQL_BLOCKING_WRAPPER(mysql_error, mysql_real_query)
  LI_MYSQL_BLOCKING_WRAPPER(mysql_error, mysql_free_result)
  LI_MYSQL_BLOCKING_WRAPPER(mysql_stmt_error, mysql_stmt_execute)
  LI_MYSQL_BLOCKING_WRAPPER(mysql_stmt_error, mysql_stmt_reset)
  LI_MYSQL_BLOCKING_WRAPPER(mysql_stmt_error, mysql_stmt_prepare)
  LI_MYSQL_BLOCKING_WRAPPER(mysql_stmt_error, mysql_stmt_fetch)
  LI_MYSQL_BLOCKING_WRAPPER(mysql_stmt_error, mysql_stmt_free_result)
  LI_MYSQL_BLOCKING_WRAPPER(mysql_stmt_error, mysql_stmt_store_result)

#undef LI_MYSQL_BLOCKING_WRAPPER

};

// Non blocking version.
template <typename Y> struct mysql_functions_non_blocking {
  enum { is_blocking = false };

  template <typename RT, typename A1, typename... A, typename B1, typename... B>
  auto mysql_non_blocking_call(std::shared_ptr<int> connection_status,
                              const char* fn_name, 
                               const char *error_fun(B1),
                               int fn_start(RT*, B1, B...),
                               int fn_cont(RT*, B1, int), A1&& a1, A&&... args) {

    RT ret;
    int status = fn_start(&ret, std::forward<A1>(a1), std::forward<A>(args)...);

    bool error = false;
    while (status) {
      try {
        fiber_.yield();
      } catch (typename Y::exception_type& e) {
        // Yield thrown a exception (probably because a closed connection).
        // Mark the connection as broken because it is left in a undefined state.
        *connection_status = 1;
        throw std::move(e);
      }

      status = fn_cont(&ret, std::forward<A1>(a1), status);
    }
    if (ret and ret != MYSQL_NO_DATA and ret != MYSQL_DATA_TRUNCATED)
    {
      *connection_status = 1;
      throw std::runtime_error(std::string("Mysql error in ") + fn_name + ": " + error_fun(a1));
    }
    return ret;
  }


#define LI_MYSQL_NONBLOCKING_WRAPPER(ERR, FN)                                                           \
  template <typename... A> auto FN(std::shared_ptr<int> connection_status, A&&... a) {                                                     \
    return mysql_non_blocking_call(connection_status, #FN, ERR, ::FN##_start, ::FN##_cont, std::forward<A>(a)...);              \
  }

  LI_MYSQL_NONBLOCKING_WRAPPER(mysql_error, mysql_fetch_row)
  LI_MYSQL_NONBLOCKING_WRAPPER(mysql_error, mysql_real_query)
  LI_MYSQL_NONBLOCKING_WRAPPER(mysql_error, mysql_free_result)
  LI_MYSQL_NONBLOCKING_WRAPPER(mysql_stmt_error, mysql_stmt_execute)
  LI_MYSQL_NONBLOCKING_WRAPPER(mysql_stmt_error, mysql_stmt_reset)
  LI_MYSQL_NONBLOCKING_WRAPPER(mysql_stmt_error, mysql_stmt_prepare)
  LI_MYSQL_NONBLOCKING_WRAPPER(mysql_stmt_error, mysql_stmt_fetch)
  LI_MYSQL_NONBLOCKING_WRAPPER(mysql_stmt_error, mysql_stmt_free_result)
  LI_MYSQL_NONBLOCKING_WRAPPER(mysql_stmt_error, mysql_stmt_store_result)

#undef LI_MYSQL_NONBLOCKING_WRAPPER

  Y& fiber_;
};

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_ASYNC_WRAPPER

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_STATEMENT
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_STATEMENT




namespace li {

  auto type_to_mysql_statement_buffer_type(const char&) { return MYSQL_TYPE_TINY; }
auto type_to_mysql_statement_buffer_type(const short int&) { return MYSQL_TYPE_SHORT; }
auto type_to_mysql_statement_buffer_type(const int&) { return MYSQL_TYPE_LONG; }
auto type_to_mysql_statement_buffer_type(const long long int&) { return MYSQL_TYPE_LONGLONG; }
auto type_to_mysql_statement_buffer_type(const float&) { return MYSQL_TYPE_FLOAT; }
auto type_to_mysql_statement_buffer_type(const double&) { return MYSQL_TYPE_DOUBLE; }
auto type_to_mysql_statement_buffer_type(const sql_blob&) { return MYSQL_TYPE_BLOB; }
auto type_to_mysql_statement_buffer_type(const char*) { return MYSQL_TYPE_STRING; }
template <unsigned S> auto type_to_mysql_statement_buffer_type(const sql_varchar<S>) {
  return MYSQL_TYPE_STRING;
}

auto type_to_mysql_statement_buffer_type(const unsigned char&) { return MYSQL_TYPE_TINY; }
auto type_to_mysql_statement_buffer_type(const unsigned short int&) { return MYSQL_TYPE_SHORT; }
auto type_to_mysql_statement_buffer_type(const unsigned int&) { return MYSQL_TYPE_LONG; }
auto type_to_mysql_statement_buffer_type(const unsigned long long int&) {
  return MYSQL_TYPE_LONGLONG;
}


struct mysql_statement_data : std::enable_shared_from_this<mysql_statement_data> {

  MYSQL_STMT* stmt_ = nullptr;
  int num_fields_ = -1;
  MYSQL_RES* metadata_ = nullptr;
  MYSQL_FIELD* fields_ = nullptr;

  mysql_statement_data(MYSQL_STMT* stmt) {
      stmt_ = stmt;
      metadata_ = mysql_stmt_result_metadata(stmt_);
      if (metadata_)
      {
        fields_ = mysql_fetch_fields(metadata_);
        num_fields_ = mysql_num_fields(metadata_);
      }
  }

  ~mysql_statement_data() {
    if (metadata_)
      mysql_free_result(metadata_);
    mysql_stmt_free_result(stmt_);
    mysql_stmt_close(stmt_);
  }

};

// Bind parameter functions
// Used to bind input parameters of prepared statement.
template <unsigned N>
struct mysql_bind_data
{
  mysql_bind_data() {  
    memset(bind.data(), 0, N * sizeof(MYSQL_BIND)); 
  }
  std::array<unsigned long, N> real_lengths;
  std::array<MYSQL_BIND, N> bind;
};

template <typename V> void mysql_bind_param(MYSQL_BIND& b, V& v) {
  b.buffer = const_cast<std::remove_const_t<V>*>(&v);
  b.buffer_type = type_to_mysql_statement_buffer_type(v);
  b.is_unsigned = std::is_unsigned<V>::value;
}

void mysql_bind_param(MYSQL_BIND& b, std::string& s) {
  b.buffer = &s[0];
  b.buffer_type = MYSQL_TYPE_STRING;
  b.buffer_length = s.size();
}
void mysql_bind_param(MYSQL_BIND& b, const std::string& s) { mysql_bind_param(b, *const_cast<std::string*>(&s)); }

template <unsigned SIZE> void mysql_bind_param(MYSQL_BIND& b, const sql_varchar<SIZE>& s) {
  mysql_bind_param(b, *const_cast<std::string*>(static_cast<const std::string*>(&s)));
}

void mysql_bind_param(MYSQL_BIND& b, char* s) {
  b.buffer = s;
  b.buffer_type = MYSQL_TYPE_STRING;
  b.buffer_length = strlen(s);
}
void mysql_bind_param(MYSQL_BIND& b, const char* s) { mysql_bind_param(b, const_cast<char*>(s)); }

void mysql_bind_param(MYSQL_BIND& b, sql_blob& s) {
  b.buffer = &s[0];
  b.buffer_type = MYSQL_TYPE_BLOB;
  b.buffer_length = s.size();
}
void mysql_bind_param(MYSQL_BIND& b, const sql_blob& s) { mysql_bind_param(b, *const_cast<sql_blob*>(&s)); }

void mysql_bind_param(MYSQL_BIND& b, sql_null_t n) { b.buffer_type = MYSQL_TYPE_NULL; }

//
// Bind output function.
// Used to bind output values to result sets.
//
template <typename T> void mysql_bind_output(MYSQL_BIND& b, unsigned long* real_length, T& v) {
  // Default to mysql_bind_param.
  mysql_bind_param(b, v);
}

void mysql_bind_output(MYSQL_BIND& b, unsigned long* real_length, std::string& v) {
  v.resize(100);
  b.buffer_type = MYSQL_TYPE_STRING;
  b.buffer_length = v.size();
  b.buffer = &v[0];
  b.length = real_length;
}

template <unsigned SIZE>
void mysql_bind_output(MYSQL_BIND& b, unsigned long* real_length, sql_varchar<SIZE>& s) {
  s.resize(SIZE);
  b.buffer = &s[0];
  b.buffer_type = MYSQL_TYPE_STRING;
  b.buffer_length = s.size();
  b.length = real_length;
}

template <typename A> mysql_bind_data<1> mysql_bind_output(mysql_statement_data& data, A& o) {
  if (data.num_fields_ != 1)
    throw std::runtime_error("mysql_statement error: The number of column in the result set "
                              "shoud be 1. Use std::tuple or li::sio to fetch several columns or "
                              "modify the request so that it returns a set of 1 column.");

  mysql_bind_data<1> bind_data;
  mysql_bind_output(bind_data.bind[0], &bind_data.real_lengths[0], o);
  return bind_data;
}

template <typename... A>
mysql_bind_data<sizeof...(A)>
mysql_bind_output(mysql_statement_data& data, metamap<A...>& o) {
  if (data.num_fields_ != sizeof...(A)) {
    throw std::runtime_error(
        "mysql_statement error: Not enough columns in the result set to fill the object.");
  }

  mysql_bind_data<sizeof...(A)> bind_data;
  MYSQL_BIND* bind = bind_data.bind.data(); 
  unsigned long* real_lengths = bind_data.real_lengths.data(); 

  li::map(o, [&](auto k, auto& v) {
    // Find li::symbol_string(k) position.
    for (int i = 0; i < data.num_fields_; i++)
      if (!strcmp(data.fields_[i].name, li::symbol_string(k)))
      // bind the column.
      {
        mysql_bind_output(bind[i], real_lengths + i, v);
      }
  });

  for (int i = 0; i < data.num_fields_; i++) {
    if (!bind[i].buffer_type) {
      std::ostringstream ss;
      ss << "Error while binding the mysql request to a metamap object: " << std::endl
          << "   Field " << data.fields_[i].name << " could not be bound." << std::endl;
      throw std::runtime_error(ss.str());
    }
  }

  return bind_data;
}

template <typename... A>
mysql_bind_data<sizeof...(A)>
mysql_bind_output(mysql_statement_data& data, std::tuple<A...>& o) {
  if (data.num_fields_ != sizeof...(A))
    throw std::runtime_error("mysql_statement error: The number of column in the result set does "
                              "not match the number of attributes of the tuple to bind.");

  mysql_bind_data<sizeof...(A)> bind_data;
  MYSQL_BIND* bind = bind_data.bind.data(); 
  unsigned long* real_lengths = bind_data.real_lengths.data(); 

  int i = 0;
  tuple_map(o, [&](auto& m) {
    mysql_bind_output(bind[i], real_lengths + i, m);
    i++;
  });

  return bind_data;
}

template <typename B>
struct mysql_statement_result {

  long long int affected_rows() { return mysql_stmt_affected_rows(data_.stmt_); }

  template <typename T> void fetch_column(MYSQL_BIND*, unsigned long, T&, int) {}
  void fetch_column(MYSQL_BIND* b, unsigned long real_length, std::string& v, int i) {
    if (real_length <= v.size())
    {
      v.resize(real_length);
      return;
    }

    v.resize(real_length);
    b[i].buffer_length = real_length;
    b[i].length = nullptr;
    b[i].buffer = &v[0];
    if (mysql_stmt_fetch_column(data_.stmt_, b + i, i, 0) != 0)
    {
      *connection_status_ = 1;
      throw std::runtime_error(std::string("mysql_stmt_fetch_column error: ") +
                               mysql_stmt_error(data_.stmt_));
    }
  }
  template <unsigned SIZE>
  void fetch_column(MYSQL_BIND& b, unsigned long real_length, sql_varchar<SIZE>& v, int i) {
    v.resize(real_length);
  }

  template <typename T> int fetch(T&& o) {

    auto bind_data = mysql_bind_output(data_, o);
    unsigned long* real_lengths = bind_data.real_lengths.data();
    MYSQL_BIND* bind = bind_data.bind.data();
    
    mysql_stmt_bind_result(data_.stmt_, bind);

    int f = mysql_wrapper_.mysql_stmt_fetch(connection_status_, data_.stmt_);
    int res = 1;
    if (f == MYSQL_NO_DATA)
      res = 0;
    else
      finalize_fetch(bind, real_lengths, o);

    mysql_wrapper_.mysql_stmt_free_result(connection_status_, data_.stmt_);
    return res;
  }
  template <typename T> void read(std::optional<T>& o) {
    T t;
    if (fetch(t))
      o = std::optional<T>(t);
    else
      o = std::optional<T>();
  }

  template <typename T> T read() {
    T t;
    if (!fetch(t))
      throw std::runtime_error("Request did not return any data.");
    return t;
  }

  template <typename T> std::optional<T> read_optional() {
    T t;
    if (fetch(t))
      return std::optional<T>(t);
    else
      return std::optional<T>();
  }

  template <typename T>
  struct unconstref_tuple_elements {};
  template <typename... T>
  struct unconstref_tuple_elements<std::tuple<T...>> {
    typedef std::tuple<std::remove_const_t<std::remove_reference_t<T>>...> ret;
  };

  template <typename F> void map(F f) {

    typedef typename unconstref_tuple_elements<callable_arguments_tuple_t<F>>::ret tp;
    typedef std::remove_const_t<std::remove_reference_t<std::tuple_element_t<0, tp>>> T;

    auto o = []() { if constexpr (li::is_metamap<T>::value) return T{}; else return tp{}; }();

    auto bind_data = mysql_bind_output(data_, o);
    mysql_stmt_bind_result(data_.stmt_, bind_data.bind.data());
    
    while (mysql_wrapper_.mysql_stmt_fetch(connection_status_, data_.stmt_) != MYSQL_NO_DATA) {
      this->finalize_fetch(bind_data.bind.data(), bind_data.real_lengths.data(), o);
      if constexpr (li::is_metamap<T>::value) 
        f(o);
      else apply(o, f);
    }

    mysql_wrapper_.mysql_stmt_free_result(connection_status_, data_.stmt_);
  }


  template <typename... A>
  void finalize_fetch(MYSQL_BIND* bind, unsigned long* real_lengths, metamap<A...>& o) {
    li::map(o, [&](auto k, auto& v) {
      for (int i = 0; i < data_.num_fields_; i++)
        if (!strcmp(data_.fields_[i].name, li::symbol_string(k)))
          this->fetch_column(bind, real_lengths[i], v, i);
    });
  }

  template <typename... A>
  void finalize_fetch(MYSQL_BIND* bind, unsigned long* real_lengths, std::tuple<A...>& o) {
    int i = 0;
    tuple_map(o, [&](auto& m) {
      this->fetch_column(bind, real_lengths[i], m, i);
      i++;
    });
  }

  template <typename A> void finalize_fetch(MYSQL_BIND* bind, unsigned long* real_lengths, A& o) {
    this->fetch_column(bind, real_lengths[0], o, 0);
  }

  void wait () {}

  long long int last_insert_id() { return mysql_stmt_insert_id(data_.stmt_); }

  bool empty() { return mysql_wrapper_.mysql_stmt_fetch(connection_status_, data_.stmt_) == MYSQL_NO_DATA; }

  B& mysql_wrapper_;
  mysql_statement_data& data_;
  std::shared_ptr<int> connection_status_;
};

template <typename B>
struct mysql_statement {

  mysql_statement_result<B> operator()() {
    mysql_wrapper_.mysql_stmt_execute(connection_status_, data_.stmt_);
    return mysql_statement_result<B>{mysql_wrapper_, data_, connection_status_};
  }

  template <typename... T> auto operator()(T&&... args) {
    MYSQL_BIND bind[sizeof...(T)];
    memset(bind, 0, sizeof(bind));

    int i = 0;
    tuple_map(std::forward_as_tuple(args...), [&](auto& m) {
      mysql_bind_param(bind[i], m);
      i++;
    });

    if (mysql_stmt_bind_param(data_.stmt_, bind) != 0)
    {
      *connection_status_ = 1;
      throw std::runtime_error(std::string("mysql_stmt_bind_param error: ") +
                               mysql_stmt_error(data_.stmt_));
    }
    mysql_wrapper_.mysql_stmt_execute(connection_status_, data_.stmt_);
    return mysql_statement_result<B>{mysql_wrapper_, data_, connection_status_};
  }

  B& mysql_wrapper_;
  mysql_statement_data& data_;
  std::shared_ptr<int> connection_status_;
};

}

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_STATEMENT


namespace li {

int max_mysql_connections_per_thread = 200;

struct mysql_tag {};

struct mysql_database;

struct mysql_connection_data {

  ~mysql_connection_data() {
    mysql_close(connection);
  }

  MYSQL* connection;
  std::unordered_map<std::string, std::shared_ptr<mysql_statement_data>> statements;
  type_hashmap<std::shared_ptr<mysql_statement_data>> statements_hashmap;
};

thread_local std::deque<std::shared_ptr<mysql_connection_data>> mysql_connection_pool;
thread_local std::deque<std::shared_ptr<mysql_connection_data>> mysql_connection_async_pool;
thread_local int total_number_of_mysql_connections = 0;

template <typename B> // must be mysql_functions_blocking or mysql_functions_non_blocking
struct mysql_result {

  B& mysql_wrapper_;
  MYSQL* con_;
  std::shared_ptr<int> connection_status_;
  MYSQL_RES* result_ = nullptr;

  template <typename T>
  T read() {
    result_ = mysql_use_result(con_);
    MYSQL_ROW row = mysql_wrapper_.mysql_fetch_row(connection_status_, result_);
    return boost::lexical_cast<T>(row[0]);
    mysql_wrapper_.mysql_free_result(connection_status_, result_); 
  } 

  void wait() {}

};

template <typename B> // must be mysql_functions_blocking or mysql_functions_non_blocking
struct mysql_connection {

  typedef mysql_tag db_tag;

  inline mysql_connection(B mysql_wrapper, std::shared_ptr<li::mysql_connection_data> data)
      : mysql_wrapper_(mysql_wrapper), data_(data), stm_cache_(data->statements),
        con_(data->connection){

    connection_status_ = std::shared_ptr<int>(new int(0), [=](int* p) mutable { 

      if (*p or (mysql_connection_pool.size() + mysql_connection_async_pool.size())  > max_mysql_connections_per_thread)
      {
      //  if constexpr (!B::is_blocking)
      //    mysql_wrapper.yield_.unsubscribe(mysql_get_socket(data->connection));
       total_number_of_mysql_connections--;
        //std::cerr << "Discarding broken mysql connection." << std::endl;
        return;
      }
      
      if constexpr (B::is_blocking)
        mysql_connection_pool.push_back(data);
      else  
        mysql_connection_async_pool.push_back(data);

    });
  }


  long long int last_insert_rowid() { return mysql_insert_id(con_); }

  mysql_result<B> operator()(const std::string& rq) { 
    mysql_wrapper_.mysql_real_query(connection_status_, con_, rq.c_str(), rq.size());
    return mysql_result<B>{mysql_wrapper_, con_, connection_status_};
  }

  template <typename F, typename... K>
  mysql_statement<B> cached_statement(F f, K... keys) {
    if (data_->statements_hashmap(f).get() == nullptr)
    {
      mysql_statement<B> res = prepare(f());
      data_->statements_hashmap(f, keys...) = res.data_.shared_from_this();
      return res;
    }
    else
      return mysql_statement<B>{mysql_wrapper_, 
                                *data_->statements_hashmap(f, keys...), 
                                 connection_status_};
  }

  mysql_statement<B> prepare(const std::string& rq) {
    auto it = stm_cache_.find(rq);
    if (it != stm_cache_.end())
    {
      //mysql_wrapper_.mysql_stmt_free_result(it->second->stmt_);
      //mysql_wrapper_.mysql_stmt_reset(it->second->stmt_);
      return mysql_statement<B>{mysql_wrapper_, *it->second, connection_status_};
    }
    //std::cout << "prepare " << rq << std::endl;
    MYSQL_STMT* stmt = mysql_stmt_init(con_);
    if (!stmt)
    {
      *connection_status_ = true;
      throw std::runtime_error(std::string("mysql_stmt_init error: ") + mysql_error(con_));
    }
    if (mysql_wrapper_.mysql_stmt_prepare(connection_status_, stmt, rq.data(), rq.size()))
    {
      *connection_status_ = true;
      throw std::runtime_error(std::string("mysql_stmt_prepare error: ") + mysql_error(con_));
    }

    auto pair = stm_cache_.emplace(rq, std::make_shared<mysql_statement_data>(stmt));
    return mysql_statement<B>{mysql_wrapper_, *pair.first->second, connection_status_};
  }

  template <typename T>
  inline std::string type_to_string(const T&, std::enable_if_t<std::is_integral<T>::value>* = 0) {
    return "INT";
  }
  template <typename T>
  inline std::string type_to_string(const T&,
                                    std::enable_if_t<std::is_floating_point<T>::value>* = 0) {
    return "DOUBLE";
  }
  inline std::string type_to_string(const std::string&) { return "MEDIUMTEXT"; }
  inline std::string type_to_string(const sql_blob&) { return "BLOB"; }
  template <unsigned S> inline std::string type_to_string(const sql_varchar<S>) {
    std::ostringstream ss;
    ss << "VARCHAR(" << S << ')';
    return ss.str();
  }

  B mysql_wrapper_;
  std::shared_ptr<mysql_connection_data> data_;
  std::unordered_map<std::string, std::shared_ptr<mysql_statement_data>>& stm_cache_;
  MYSQL* con_;
  std::shared_ptr<int> connection_status_;
};


struct mysql_database : std::enable_shared_from_this<mysql_database> {

  template <typename... O> inline mysql_database(O... opts) {

    auto options = mmm(opts...);
    static_assert(has_key(options, s::host), "open_mysql_connection requires the s::host argument");
    static_assert(has_key(options, s::database),
                  "open_mysql_connection requires the s::databaser argument");
    static_assert(has_key(options, s::user), "open_mysql_connection requires the s::user argument");
    static_assert(has_key(options, s::password),
                  "open_mysql_connection requires the s::password argument");

    host_ = options.host;
    database_ = options.database;
    user_ = options.user;
    passwd_ = options.password;
    port_ = get_or(options, s::port, 0);

    character_set_ = get_or(options, s::charset, "utf8");

    if (mysql_library_init(0, NULL, NULL))
      throw std::runtime_error("Could not initialize MySQL library.");
    if (!mysql_thread_safe())
      throw std::runtime_error("Mysql is not compiled as thread safe.");
  }

  ~mysql_database() { mysql_library_end(); }

  template <typename Y>
  inline mysql_connection<mysql_functions_non_blocking<Y>> connect(Y& fiber) {

    //std::cout << "nconnection " << total_number_of_mysql_connections << std::endl;
    int ntry = 0;
    std::shared_ptr<mysql_connection_data> data = nullptr;
    while (!data)
    {
      // if (ntry > 20)
      //   throw std::runtime_error("Cannot connect to the database");
      ntry++;

      if (!mysql_connection_async_pool.empty()) {
        data = mysql_connection_async_pool.back();
        mysql_connection_async_pool.pop_back();
        fiber.epoll_add(mysql_get_socket(data->connection), EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET);
      }
      else
      {
        // std::cout << total_number_of_mysql_connections << " connections. "<< std::endl;
        if (total_number_of_mysql_connections > max_mysql_connections_per_thread)
        {
          //std::cout << "Waiting for a free mysql connection..." << std::endl;
          fiber.yield();
          continue;
        }
        total_number_of_mysql_connections++;
        //std::cout << "NEW MYSQL CONNECTION "  << std::endl; 
        MYSQL* mysql;
        int mysql_fd = -1;
        int status;
        MYSQL* connection;
        //while (mysql_fd == -1)
        {
          mysql = new MYSQL;
          mysql_init(mysql);
          mysql_options(mysql, MYSQL_OPT_NONBLOCK, 0);
          connection = nullptr;
          status = mysql_real_connect_start(&connection, mysql, host_.c_str(), user_.c_str(), passwd_.c_str(),
                                            database_.c_str(), port_, NULL, 0);

          //std::cout << "after: " << mysql_get_socket(mysql) << " " << status == MYSQL_ << std::endl;
          mysql_fd = mysql_get_socket(mysql);
          if (mysql_fd == -1)
          {
             //std::cout << "Invalid mysql connection bad mysql_get_socket " << status << " " << mysql << std::endl;
            mysql_close(mysql);
            total_number_of_mysql_connections--;
            //usleep(1e6);
            fiber.yield();
            continue;
          }
        }
        if (status)
          fiber.epoll_add(mysql_fd, EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET);
        while (status) try {
          fiber.yield();
          status = mysql_real_connect_cont(&connection, mysql, status);
        }  catch (typename Y::exception_type& e) {
          // Yield thrown a exception (probably because a closed connection).
          // std::cerr << "Warning: yield threw an exception while connecting to mysql: "
          //  << total_number_of_mysql_connections << std::endl;
          total_number_of_mysql_connections--;
          mysql_close(mysql);
          throw std::move(e);
        }
        if (!connection)
        {
          //std::cout << "Error in mysql_real_connect_cont" << std::endl;
          total_number_of_mysql_connections--;
          fiber.yield();
          continue;
        }
          //throw std::runtime_error("Cannot connect to the database");
        mysql_set_character_set(mysql, character_set_.c_str());
        data = std::shared_ptr<mysql_connection_data>(new mysql_connection_data{mysql});
      }
    }
    assert(data);
    return std::move(mysql_connection(mysql_functions_non_blocking<Y>{fiber}, data));
  }

  inline mysql_connection<mysql_functions_blocking> connect() {
    std::shared_ptr<mysql_connection_data> data = nullptr;
    if (!mysql_connection_pool.empty()) {
      data = mysql_connection_pool.back();
      mysql_connection_pool.pop_back();
    }

    if (!data) {
      MYSQL* con_ = mysql_init(nullptr);
      con_ = mysql_real_connect(con_, host_.c_str(), user_.c_str(), passwd_.c_str(),
                                database_.c_str(), port_, NULL, 0);
      if (!con_)
        throw std::runtime_error("Cannot connect to the database");

      //total_number_of_mysql_connections++;
      mysql_set_character_set(con_, character_set_.c_str());
      data = std::shared_ptr<mysql_connection_data>(new mysql_connection_data{con_});
    }

    assert(data);
    return std::move(mysql_connection(mysql_functions_blocking{}, data));
  }

  std::mutex mutex_;
  std::string host_, user_, passwd_, database_;
  unsigned int port_;
  std::deque<MYSQL*> _;
  std::string character_set_;
};


} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_HTTP_BACKEND
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_HTTP_BACKEND

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_JSON
#define LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_JSON


#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_DECODER
#define LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_DECODER

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_DECODE_STRINGSTREAM
#define LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_DECODE_STRINGSTREAM

#if defined(_MSC_VER)
#endif


namespace li {

using std::string_view;

namespace internal {
template <typename I> void parse_uint(I* val_, const char* str, const char** end) {
  I& val = *val_;
  val = 0;
  int i = 0;
  while (i < 40) {
    char c = *str;
    if (c < '0' or c > '9')
      break;
    val = val * 10 + c - '0';
    str++;
    i++;
  }
  if (end)
    *end = str;
}

template <typename I> void parse_int(I* val, const char* str, const char** end) {
  bool neg = false;

  if (str[0] == '-') {
    neg = true;
    str++;
  }
  parse_uint(val, str, end);
  if constexpr (!std::is_same<I, bool>::value) {
    if (neg)
      *val = -(*val);
  }
}

inline unsigned long long pow10(unsigned int e) {
  unsigned long long pows[] = {1,
                               10,
                               100,
                               1000,
                               10000,
                               100000,
                               1000000,
                               10000000,
                               100000000,
                               1000000000,
                               10000000000,
                               100000000000,
                               1000000000000,
                               10000000000000,
                               100000000000000,
                               1000000000000000,
                               10000000000000000,
                               100000000000000000};

  if (e < 18)
    return pows[e];
  else
    return 0;
}

template <typename F> void parse_float(F* f, const char* str, const char** end) {
  // 1.234e-10
  // [sign][int][decimal_part][exp]

  const char* it = str;
  int integer_part;
  parse_int(&integer_part, it, &it);
  int sign = integer_part >= 0 ? 1 : -1;
  *f = integer_part;
  if (*it == '.') {
    it++;
    unsigned long long decimal_part;
    const char* dec_end;
    parse_uint(&decimal_part, it, &dec_end);

    if (dec_end > it)
      *f += (F(decimal_part) / pow10(dec_end - it)) * sign;

    it = dec_end;
  }

  if (*it == 'e' || *it == 'E') {
    it++;
    bool neg = false;
    if (*it == '-') {
      neg = true;
      it++;
    }

    unsigned int exp = 0;
    parse_uint(&exp, it, &it);
    if (neg)
      *f = *f / pow10(exp);
    else
      *f = *f * pow10(exp);
  }

  if (end)
    *end = it;
}

} // namespace internal

class decode_stringstream {
public:
  inline decode_stringstream(std::string_view buffer_)
      : cur(buffer_.data()), bad_(false), buffer(buffer_) {}

  inline bool eof() const { return cur > &buffer.back(); }
  inline const char peek() const { return *cur; }
  inline const char get() { return *(cur++); }
  inline int bad() const { return bad_; }
  inline int good() const { return !bad_ && !eof(); }

  template <typename T> void operator>>(T& value) {
    eat_spaces();
    if constexpr (std::is_floating_point<T>::value) {
      // Decode floating point.
      eat_spaces();
      const char* end = nullptr;
      internal::parse_float(&value, cur, &end);
      if (end == cur)
        bad_ = true;
      cur = end;
    } else if constexpr (std::is_integral<T>::value) {
      // Decode integer.
      const char* end = nullptr;
      internal::parse_int(&value, cur, &end);
      if (end == cur)
        bad_ = true;
      cur = end;
    } else if constexpr (std::is_same<T, std::string>::value) {
      // Decode UTF8 string.
      json_to_utf8(*this, value);
    } else if constexpr (std::is_same<T, string_view>::value) {
      // Decoding to stringview does not decode utf8.

      if (get() != '"') {
        bad_ = true;
        return;
      }

      const char* start = cur;
      bool escaped = false;

      while (peek() != '\0' and (escaped or peek() != '"')) {
        int nb = 0;
        while (peek() == '\\')
          nb++;

        escaped = nb % 2;
        cur++;
      }
      const char* end = cur;
      value = string_view(start, end - start);

      if (get() != '"') {
        bad_ = true;
        return;
      }
    }
  }

private:
  inline void eat_spaces() {
    while (peek() < 33)
      ++cur;
  }

  int bad_;
  const char* cur;
  std::string_view buffer; //
};

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_DECODE_STRINGSTREAM

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_ERROR
#define LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_ERROR


namespace li {

enum json_error_code { JSON_OK = 0, JSON_KO = 1 };

struct json_error {
  json_error& operator=(const json_error&) = default;
  operator bool() { return code != 0; }
  bool good() { return code == 0; }
  bool bad() { return code != 0; }
  int code;
  std::string what;
};

int make_json_error(const char* what) { return 1; }
int json_no_error() { return 0; }

static int json_ok = json_no_error();

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_ERROR

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_UNICODE
#define LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_UNICODE




#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_SYMBOLS
#define LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_SYMBOLS

#ifndef LI_SYMBOL_append
#define LI_SYMBOL_append
    LI_SYMBOL(append)
#endif

#ifndef LI_SYMBOL_json_key
#define LI_SYMBOL_json_key
    LI_SYMBOL(json_key)
#endif

#ifndef LI_SYMBOL_name
#define LI_SYMBOL_name
    LI_SYMBOL(name)
#endif

#ifndef LI_SYMBOL_type
#define LI_SYMBOL_type
    LI_SYMBOL(type)
#endif


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_SYMBOLS


namespace li {

template <typename O> inline decltype(auto) wrap_json_output_stream(O&& s) {
  return mmm(s::append = [&s](char c) { s << c; });
}

inline decltype(auto) wrap_json_output_stream(std::ostringstream& s) {
  return mmm(s::append = [&s](char c) { s << c; });
}

inline decltype(auto) wrap_json_output_stream(std::string& s) {
  return mmm(s::append = [&s](char c) { s.append(1, c); });
}

inline decltype(auto) wrap_json_input_stream(std::istringstream& s) { return s; }
inline decltype(auto) wrap_json_input_stream(std::stringstream& s) { return s; }
inline decltype(auto) wrap_json_input_stream(decode_stringstream& s) { return s; }
inline decltype(auto) wrap_json_input_stream(const std::string& s) { return decode_stringstream(s); }
inline decltype(auto) wrap_json_input_stream(const char* s) {
  return decode_stringstream(s);
}
inline decltype(auto) wrap_json_input_stream(const std::string_view& s) {
  return decode_stringstream(s);
}

namespace unicode_impl {
template <typename S, typename T> auto json_to_utf8(S&& s, T&& o);

template <typename S, typename T> auto utf8_to_json(S&& s, T&& o);
} // namespace unicode_impl

template <typename I, typename O> auto json_to_utf8(I&& i, O&& o) {
  return unicode_impl::json_to_utf8(wrap_json_input_stream(std::forward<I>(i)),
                                    wrap_json_output_stream(std::forward<O>(o)));
}

template <typename I, typename O> auto utf8_to_json(I&& i, O&& o) {
  return unicode_impl::utf8_to_json(wrap_json_input_stream(std::forward<I>(i)),
                                    wrap_json_output_stream(std::forward<O>(o)));
}

enum json_encodings { UTF32BE, UTF32LE, UTF16BE, UTF16LE, UTF8 };

// Detection of encoding depending on the pattern of the
// first fourth characters.
auto detect_encoding(char a, char b, char c, char d) {
  // 00 00 00 xx  UTF-32BE
  // xx 00 00 00  UTF-32LE
  // 00 xx 00 xx  UTF-16BE
  // xx 00 xx 00  UTF-16LE
  // xx xx xx xx  UTF-8

  if (a == 0 and b == 0)
    return UTF32BE;
  else if (c == 0 and d == 0)
    return UTF32LE;
  else if (a == 0)
    return UTF16BE;
  else if (b == 0)
    return UTF16LE;
  else
    return UTF8;
}

// The JSON RFC escapes character codepoints prefixed with a \uXXXX (7-11 bits codepoints)
// or \uXXXX\uXXXX (20 bits codepoints)

// uft8 string have 4 kinds of character representation encoding the codepoint of the character.

// 1 byte : 0xxxxxxx  -> 7 bits codepoint ASCII chars from 0x00 to 0x7F
// 2 bytes: 110xxxxx 10xxxxxx -> 11 bits codepoint
// 3 bytes: 1110xxxx 10xxxxxx 10xxxxxx -> 11 bits codepoint

// 1 and 3 bytes representation are escaped as \uXXXX with X a char in the 0-9A-F range. It
// is possible since the codepoint is less than 16 bits.

// the 4 bytes representation uses the UTF-16 surrogate pair (high and low surrogate).

// The high surrogate is in the 0xD800..0xDBFF range (HR) and
// the low surrogate is in the 0xDC00..0xDFFF range (LR).

// to encode a given 20bits codepoint c to the surrogate pair.
//  - substract 0x10000 to c
//  - separate the result in a high (first 10 bits) and low (last 10bits) surrogate.
//  - Add 0xD800 to the high surrogate
//  - Add 0xDC00 to the low surrogate
//  - the 32 bits code is (high << 16) + low.

// and to json-escape the high-low(H-L) surrogates representation (16+16bits):
//  - Check that H and L are respectively in the HR and LR ranges.
//  - add to H-L 0x0001_0000 - 0xD800_DC00 to get the 20bits codepoint c.
//  - Encode the codepoint in a string of \uXXXX\uYYYY with X and Y the respective hex digits
//    of the high and low sequence of 10 bits.

// In addition to utf8, JSON escape characters ( " \ / ) with a backslash and translate
// \n \r \t \b \r in their matching two characters string, for example '\n' to  '\' followed by 'n'.

namespace unicode_impl {
template <typename S, typename T> auto json_to_utf8(S&& s, T&& o) {
  // Convert a JSON string into an UTF-8 string.
  if (s.get() != '"')
    return JSON_KO; // make_json_error("json_to_utf8: JSON strings should start with a double
                    // quote.");

  while (true) {
    // Copy until we find the escaping backslash or the end of the string (double quote).
    while (s.peek() != EOF and s.peek() != '"' and s.peek() != '\\')
      o.append(s.get());

    // If eof found before the end of the string, return an error.
    if (s.eof())
      return JSON_KO; // make_json_error("json_to_utf8: Unexpected end of string when parsing a
                      // string.");

    // If end of json string, return
    if (s.peek() == '"') {
      break;
      return JSON_OK;
    }

    // Get the '\'.
    assert(s.peek() == '\\');
    s.get();

    switch (s.get()) {
      // List of escaped char from http://www.json.org/
    default:
      return JSON_KO; // make_json_error("json_to_utf8: Bad JSON escaped character.");
    case '"':
      o.append('"');
      break;
    case '\\':
      o.append('\\');
      break;
    case '/':
      o.append('/');
      break;
    case 'n':
      o.append('\n');
      break;
    case 'r':
      o.append('\r');
      break;
    case 't':
      o.append('\t');
      break;
    case 'b':
      o.append('\b');
      break;
    case 'f':
      o.append('\f');
      break;
    case 'u':
      char a, b, c, d;

      a = s.get();
      b = s.get();
      c = s.get();
      d = s.get();

      if (s.eof())
        return JSON_KO; // make_json_error("json_to_utf8: Unexpected end of string when decoding an
                        // utf8 character");

      auto decode_hex_c = [](char c) {
        if (c >= '0' and c <= '9')
          return c - '0';
        else
          return (10 + std::toupper(c) - 'A');
      };

      uint16_t x = (decode_hex_c(a) << 12) + (decode_hex_c(b) << 8) + (decode_hex_c(c) << 4) +
                   decode_hex_c(d);

      // If x in the  0xD800..0xDBFF range -> decode a surrogate pair \uXXXX\uYYYY -> 20 bits
      // codepoint.
      if (x >= 0xD800 and x <= 0xDBFF) {
        if (s.get() != '\\' or s.get() != 'u')
          return JSON_KO; // make_json_error("json_to_utf8: Missing low surrogate.");

        uint16_t y = (decode_hex_c(s.get()) << 12) + (decode_hex_c(s.get()) << 8) +
                     (decode_hex_c(s.get()) << 4) + decode_hex_c(s.get());

        if (s.eof())
          return JSON_KO; // make_json_error("json_to_utf8: Unexpected end of string when decoding
                          // an utf8 character");

        x -= 0xD800;
        y -= 0xDC00;

        int cp = (x << 10) + y + 0x10000;

        o.append(0b11110000 | (cp >> 18));
        o.append(0b10000000 | ((cp & 0x3F000) >> 12));
        o.append(0b10000000 | ((cp & 0x00FC0) >> 6));
        o.append(0b10000000 | (cp & 0x003F));

      }
      // else encode the codepoint with the 1-2, or 3 bytes utf8 representation.
      else {
        if (x <= 0x007F) // 7bits codepoints, ASCII 0xxxxxxx.
        {
          o.append(uint8_t(x));
        } else if (x >= 0x0080 and x <= 0x07FF) // 11bits codepoint -> 110xxxxx 10xxxxxx
        {
          o.append(0b11000000 | (x >> 6));
          o.append(0b10000000 | (x & 0x003F));
        } else if (x >= 0x0800 and x <= 0xFFFF) // 16bits codepoint -> 1110xxxx 10xxxxxx 10xxxxxx
        {
          o.append(0b11100000 | (x >> 12));
          o.append(0b10000000 | ((x & 0x0FC0) >> 6));
          o.append(0b10000000 | (x & 0x003F));
        } else
          return JSON_KO; // make_json_error("json_to_utf8: Bad UTF8 codepoint.");
      }
      break;
    }
  }

  if (s.get() != '"')
    return JSON_KO; // make_json_error("JSON strings must end with a double quote.");

  return JSON_OK; // json_no_error();
}

template <typename S, typename T> auto utf8_to_json(S&& s, T&& o) {
  o.append('"');

  auto encode_16bits = [&](uint16_t b) {
    const char lt[] = "0123456789ABCDEF";
    o.append(lt[b >> 12]);
    o.append(lt[(b & 0x0F00) >> 8]);
    o.append(lt[(b & 0x00F0) >> 4]);
    o.append(lt[b & 0x000F]);
  };

  while (!s.eof()) {
    // 7-bits codepoint
    while (s.good() and s.peek() <= 0x7F and s.peek() != EOF) {
      switch (s.peek()) {
      case '"':
        o.append('\\');
        o.append('"');
        break;
      case '\\':
        o.append('\\');
        o.append('\\');
        break;
        // case '/': o.append('/'); break; Do not escape /
      case '\n':
        o.append('\\');
        o.append('n');
        break;
      case '\r':
        o.append('\\');
        o.append('r');
        break;
      case '\t':
        o.append('\\');
        o.append('t');
        break;
      case '\b':
        o.append('\\');
        o.append('b');
        break;
      case '\f':
        o.append('\\');
        o.append('f');
        break;
      default:
        o.append(s.peek());
      }
      s.get();
    }

    if (s.eof())
      break;

    // uft8 prefix \u.
    o.append('\\');
    o.append('u');

    uint8_t c1 = s.get();
    uint8_t c2 = s.get();
    {
      // extract codepoints.
      if (c1 < 0b11100000) // 11bits - 2 char: 110xxxxx	10xxxxxx
      {
        uint16_t cp = ((c1 & 0b00011111) << 6) + (c2 & 0b00111111);
        if (cp >= 0x0080 and cp <= 0x07FF)
          encode_16bits(cp);
        else
          return JSON_KO;         // make_json_error("utf8_to_json: Bad UTF8 codepoint.");
      } else if (c1 < 0b11110000) // 16 bits - 3 char: 1110xxxx	10xxxxxx	10xxxxxx
      {
        uint16_t cp = ((c1 & 0b00001111) << 12) + ((c2 & 0b00111111) << 6) + (s.get() & 0b00111111);

        if (cp >= 0x0800 and cp <= 0xFFFF)
          encode_16bits(cp);
        else
          return JSON_KO; // make_json_error("utf8_to_json: Bad UTF8 codepoint.");
      } else              // 21 bits - 4 chars: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
      {
        int cp = ((c1 & 0b00000111) << 18) + ((c2 & 0b00111111) << 12) +
                 ((s.get() & 0b00111111) << 6) + (s.get() & 0b00111111);

        cp -= 0x10000;

        uint16_t H = (cp >> 10) + 0xD800;
        uint16_t L = (cp & 0x03FF) + 0xDC00;

        // check if we are in the right range.
        // The high surrogate is in the 0xD800..0xDBFF range (HR) and
        // the low surrogate is in the 0xDC00..0xDFFF range (LR).
        assert(H >= 0xD800 and H <= 0xDBFF and L >= 0xDC00 and L <= 0xDFFF);

        encode_16bits(H);
        o.append('\\');
        o.append('u');
        encode_16bits(L);
      }
    }
  }
  o.append('"');
  return JSON_OK;
}
} // namespace unicode_impl
} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_UNICODE

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_UTILS
#define LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_UTILS



namespace li {

template <typename T> struct json_object_base;

template <typename T> struct json_object_;
template <typename T> struct json_vector_;
template <typename V> struct json_value_;
template <typename... T> struct json_tuple_;
struct json_key;

namespace impl {
template <typename S, typename... A>
auto make_json_object_member(const function_call_exp<S, A...>& e);
template <typename S> auto make_json_object_member(const li::symbol<S>&);

template <typename S, typename T> auto make_json_object_member(const assign_exp<S, T>& e) {
  return cat(make_json_object_member(e.left), mmm(s::type = e.right));
}

template <typename S> auto make_json_object_member(const li::symbol<S>&) {
  return mmm(s::name = S{});
}

template <typename V> auto to_json_schema(V v) { return json_value_<V>{}; }

template <typename... M> auto to_json_schema(const metamap<M...>& m);

template <typename V> auto to_json_schema(const std::vector<V>& arr) {
  auto elt = to_json_schema(decltype(arr[0]){});
  return json_vector_<decltype(elt)>{elt};
}

template <typename... V> auto to_json_schema(const std::tuple<V...>& arr) {
  return json_tuple_<decltype(to_json_schema(V{}))...>(to_json_schema(V{})...);
}

template <typename... M> auto to_json_schema(const metamap<M...>& m) {
  auto tuple_maker = [](auto&&... t) { return std::make_tuple(std::forward<decltype(t)>(t)...); };

  auto entities = map_reduce(
      m, [](auto k, auto v) { return mmm(s::name = k, s::type = to_json_schema(v)); }, tuple_maker);

  return json_object_<decltype(entities)>(entities);
}

template <typename... E> auto json_object_to_metamap(const json_object_<std::tuple<E...>>& s) {
  auto make_kvs = [](auto... elt) { return std::make_tuple((elt.name = elt.type)...); };

  auto kvs = std::apply(make_kvs, s.entities);
  return std::apply(mmm, kvs);
}

template <typename S, typename... A>
auto make_json_object_member(const function_call_exp<S, A...>& e) {
  auto res = mmm(s::name = e.method, s::json_key = symbol_string(e.method));

  auto parse = [&](auto a) {
    if constexpr (std::is_same<decltype(a), json_key>::value) {
      res.json_key = a.key;
    }
  };

  ::li::tuple_map(e.args, parse);
  return res;
}

} // namespace impl

template <typename T> struct json_object_;

template <typename O> struct json_vector_;

template <typename E> constexpr auto json_is_vector(json_vector_<E>) -> std::true_type {
  return {};
}
template <typename E> constexpr auto json_is_vector(E) -> std::false_type { return {}; }

template <typename... E> constexpr auto json_is_tuple(json_tuple_<E...>) -> std::true_type {
  return {};
}
template <typename E> constexpr auto json_is_tuple(E) -> std::false_type { return {}; }

template <typename E> constexpr auto json_is_object(json_object_<E>) -> std::true_type {
  return {};
}
template <typename E> constexpr auto json_is_object(E) -> std::false_type { return {}; }

template <typename E> constexpr auto json_is_value(json_object_<E>) -> std::false_type {
  return {};
}
template <typename E> constexpr auto json_is_value(json_vector_<E>) -> std::false_type {
  return {};
}
template <typename... E> constexpr auto json_is_value(json_tuple_<E...>) -> std::false_type {
  return {};
}
template <typename E> constexpr auto json_is_value(E) -> std::true_type { return {}; }

template <typename T> constexpr auto is_std_optional(std::optional<T>) -> std::true_type;
template <typename T> constexpr auto is_std_optional(T) -> std::false_type;

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_UTILS


namespace li {

namespace impl {

template <typename S> struct json_parser {
  inline json_parser(S&& s) : ss(s) {}
  inline json_parser(S& s) : ss(s) {}

  inline decltype(auto) peek() { return ss.peek(); }
  inline decltype(auto) get() { return ss.get(); }

  inline void skip_one() { ss.get(); }

  inline bool eof() { return ss.eof(); }
  inline json_error_code eat(char c, bool skip_spaces = true) {
    if (skip_spaces)
      eat_spaces();

    char g = ss.get();
    if (g != c)
      return make_json_error("Unexpected char. Got '", char(g), "' expected ", c);
    return JSON_OK;
  }

  inline json_error_code eat(const char* str, bool skip_spaces = true) {
    if (skip_spaces)
      eat_spaces();

    const char* str_it = str;
    while (*str_it) {
      char g = ss.get();
      if (g != *str_it)
        return make_json_error("Unexpected char. Got '", char(g), "' expected '", *str_it,
                               "' when parsing string ", str);
      str_it++;
    }
    return JSON_OK;
  }

  template <typename... T> inline json_error_code make_json_error(T&&... t) {
    if (!error_stream)
      error_stream = new std::ostringstream();
    *error_stream << "json error: ";
    auto add = [this](auto w) { *error_stream << w; };
    apply_each(add, t...);
    return JSON_KO;
  }
  inline void eat_spaces() {
    while (ss.peek() >= 0 and ss.peek() < 33)
      ss.get();
  }

  template <typename X> struct JSON_INVALID_TYPE;

  // Integers and floating points.
  template <typename T> json_error_code fill(T& t) {

    if constexpr (std::is_floating_point<T>::value or std::is_integral<T>::value or
                  std::is_same<T, std::string_view>::value) {
      ss >> t;
      if (ss.bad())
        return make_json_error("Ill-formated value.");
      return JSON_OK;
    } else
      // The JSON decoder only parses floating-point, integral and string types.
      return JSON_INVALID_TYPE<T>::error;
  }

  // Strings
  inline json_error_code fill(std::string& str) {
    eat_spaces();
    str.clear();
    return json_to_utf8(ss, str);
  }

  template <typename T> inline json_error_code fill(std::optional<T>& opt) {
    opt.emplace();
    return fill(opt.value());
  }

  template <typename... T> inline json_error_code fill(std::variant<T...>& v) {
    if (auto err = eat('{'))
      return err;
    if (auto err = eat("\"idx\""))
      return err;
    if (auto err = eat(':'))
      return err;

    int idx = 0;
    fill(idx);
    if (auto err = eat(','))
      return err;
    if (auto err = eat("\"value\""))
      return err;
    if (auto err = eat(':'))
      return err;

    int cpt = 0;
    apply_each(
        [&](auto* x) {
          if (cpt == idx) {
            std::remove_pointer_t<decltype(x)> value{};
            fill(value);
            v = std::move(value);
          }
          cpt++;
        },
        (T*)nullptr...);

    if (auto err = eat('}'))
      return err;
    return JSON_OK;
  }

  S& ss;
  std::ostringstream* error_stream = nullptr;
};

template <typename P, typename O, typename S> json_error_code json_decode2(P& p, O& obj, S) {
  auto err = p.fill(obj);
  if (err)
    return err;
  else
    return JSON_OK;
}

template <typename P, typename O, typename S>
json_error_code json_decode2(P& p, O& obj, json_vector_<S> schema) {
  obj.clear();
  bool first = true;
  auto err = p.eat('[');
  if (err)
    return err;

  p.eat_spaces();
  while (p.peek() != ']') {
    if (!first) {
      if ((err = p.eat(',')))
        return err;
    }
    first = false;

    obj.resize(obj.size() + 1);
    if ((err = json_decode2(p, obj.back(), S{})))
      return err;
    p.eat_spaces();
  }

  if ((err = p.eat(']')))
    return err;
  else
    return JSON_OK;
}

template <typename F, typename... E, typename... T, std::size_t... I>
inline void json_decode_tuple_elements(F& decode_fun, std::tuple<T...>& tu,
                                       const std::tuple<E...>& schema, std::index_sequence<I...>) {
  (void)std::initializer_list<int>{((void)decode_fun(std::get<I>(tu), std::get<I>(schema)), 0)...};
}

template <typename P, typename... O, typename... S>
json_error_code json_decode2(P& p, std::tuple<O...>& tu, json_tuple_<S...> schema) {
  bool first = true;
  auto err = p.eat('[');
  if (err)
    return err;

  auto decode_one_element = [&first, &p, &err](auto& value, auto value_schema) {
    if (!first) {
      if ((err = p.eat(',')))
        return err;
    }
    first = false;
    if ((err = json_decode2(p, value, value_schema)))
      return err;
    p.eat_spaces();
    return JSON_OK;
  };

  json_decode_tuple_elements(decode_one_element, tu, schema.elements,
                             std::make_index_sequence<sizeof...(O)>{});

  if ((err = p.eat(']')))
    return err;
  else
    return JSON_OK;
}

template <typename P, typename O, typename S>
json_error_code json_decode2(P& p, O& obj, json_object_<S> schema) {
  json_error_code err;
  if ((err = p.eat('{')))
    return err;

  struct attr_info {
    bool filled;
    bool required;
    const char* name;
    int name_len;
    std::function<json_error_code(P&)> parse_value;
  };
  constexpr int n_members = std::tuple_size<decltype(schema.schema)>();
  attr_info A[n_members];
  int i = 0;
  auto prepare = [&](auto m) {
    A[i].filled = false;
    A[i].required = true;
    A[i].name = symbol_string(m.name);
    A[i].name_len = strlen(symbol_string(m.name));

    if constexpr (has_key(m, s::json_key)) {
      A[i].name = m.json_key;
    }

    if constexpr (decltype(is_std_optional(symbol_member_or_getter_access(obj, m.name))){}) {
      A[i].required = false;
    }

    A[i].parse_value = [m, &obj](P& p) {
      using V = decltype(symbol_member_or_getter_access(obj, m.name));
      using VS = decltype(get_or(m, s::type, json_value_<V>{}));

      if constexpr (decltype(json_is_value(VS{})){}) {
        if (auto err = p.fill(symbol_member_or_getter_access(obj, m.name)))
          return err;
        else
          return JSON_OK;
      } else {
        if (auto err = json_decode2(p, symbol_member_or_getter_access(obj, m.name), m.type))
          return err;
        else
          return JSON_OK;
      }
    };

    i++;
  };

  std::apply([&](auto... m) { apply_each(prepare, m...); }, schema.schema);

  while (p.peek() != '}') {

    bool found = false;
    if ((err = p.eat('"')))
      return err;
    char symbol[50];
    int symbol_size = 0;
    while (!p.eof() and p.peek() != '"' and symbol_size < 50)
      symbol[symbol_size++] = p.get();
    symbol[symbol_size] = 0;
    if ((err = p.eat('"', false)))
      return err;

    for (int i = 0; i < n_members; i++) {
      int len = A[i].name_len;
      if (!strncmp(symbol, A[i].name, len)) {
        if ((err = p.eat(':')))
          return err;
        if (A[i].filled)
          return p.make_json_error("Duplicate json key: ", A[i].name);

        if ((err = A[i].parse_value(p)))
          return err;
        A[i].filled = true;
        found = true;
        break;
      }
    }

    if (!found)
      return p.make_json_error("Unknown json key");
    p.eat_spaces();
    if (p.peek() == ',') {
      if ((err = p.eat(',')))
        return err;
    }
  }
  if ((err = p.eat('}')))
    return err;

  for (int i = 0; i < n_members; i++) {
    if (A[i].required and !A[i].filled)
      return p.make_json_error("Missing json key ", A[i].name);
  }
  return JSON_OK;
}

template <typename C, typename O, typename S> json_error json_decode(C& input, O& obj, S schema) {
  auto stream = decode_stringstream(input);
  json_parser<decode_stringstream> p(stream);
  if (json_decode2(p, obj, schema))
    return json_error{1, p.error_stream ? p.error_stream->str() : "Json error"};
  else
    return json_error{0};
}

} // namespace impl

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_DECODER

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_ENCODER
#define LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_ENCODER


namespace li {

using std::string_view;

template <typename... T> struct json_tuple_;
template <typename T> struct json_object_;

namespace impl {

// Json encoder.
// =============================================

template <typename C, typename O, typename E>
inline void json_encode(C& ss, O obj, const json_object_<E>& schema);
template <typename C, typename... E, typename... T>
inline void json_encode(C& ss, const std::tuple<T...>& tu, const json_tuple_<E...>& schema);
template <typename T, typename C, typename E>
inline void json_encode(C& ss, const T& value, const E& schema);
template <typename T, typename C, typename E>
inline void json_encode(C& ss, const std::vector<T>& array, const json_vector_<E>& schema);

template <typename T, typename C> inline void json_encode_value(C& ss, const T& t) { ss << t; }

template <typename C> inline void json_encode_value(C& ss, const char* s) { 
  //ss << s;
  utf8_to_json(s, ss);
 }

template <typename C> inline void json_encode_value(C& ss, const string_view& s) {
  //ss << s;
  utf8_to_json(s, ss);
}

template <typename C> inline void json_encode_value(C& ss, const std::string& s) {
  //ss << s;
  utf8_to_json(s, ss);
}

template <typename C, typename... T> inline void json_encode_value(C& ss, const metamap<T...>& s) {
  json_encode(ss, s, to_json_schema(s));
}

template <typename T, typename C> inline void json_encode_value(C& ss, const std::optional<T>& t) {
  if (t.has_value())
    json_encode_value(ss, t.value());
}

template <typename C, typename... T>
inline void json_encode_value(C& ss, const std::variant<T...>& t) {
  ss << "{\"idx\":" << t.index() << ",\"value\":";
  std::visit([&](auto&& value) { json_encode_value(ss, value); }, t);
  ss << '}';
}

template <typename C, typename O, typename E>
inline void json_encode(C& ss, O obj, const json_object_<E>& schema);

template <typename T, typename C, typename E>
inline void json_encode(C& ss, const T& value, const E& schema) {
  json_encode_value(ss, value);
}

template <typename T, typename C, typename E>
inline void json_encode(C& ss, const std::vector<T>& array, const json_vector_<E>& schema) {
  ss << '[';
  for (const auto& t : array) {
    if constexpr (decltype(json_is_vector(E{})){} or decltype(json_is_object(E{})){}) {
      json_encode(ss, t, schema.schema);
    } else
      json_encode_value(ss, t);

    if (&t != &array.back())
      ss << ',';
  }
  ss << ']';
}

template <typename F, typename... E, typename... T, std::size_t... I>
inline void json_encode_tuple_elements(F& encode_fun, const std::tuple<T...>& tu,
                                       const std::tuple<E...>& schema, std::index_sequence<I...>) {
  (void)std::initializer_list<int>{((void)encode_fun(std::get<I>(tu), std::get<I>(schema)), 0)...};
}

template <typename C, typename... E, typename... T>
inline void json_encode(C& ss, const std::tuple<T...>& tu, const json_tuple_<E...>& schema) {
  ss << '[';
  bool first = true;
  auto encode_one_element = [&first, &ss](auto value, auto value_schema) {
    if (!first)
      ss << ',';
    first = false;
    if constexpr (decltype(json_is_value(value_schema)){}) {
      json_encode_value(ss, value);
    } else
      json_encode(ss, value, value_schema);
  };

  json_encode_tuple_elements(encode_one_element, tu, schema.elements,
                             std::make_index_sequence<sizeof...(T)>{});
  ss << ']';
}

template <typename C, typename O, typename E>
inline void json_encode(C& ss, O obj, const json_object_<E>& schema) {
  ss << '{';
  bool first = true;

  auto encode_one_entity = [&](auto e) {
    if constexpr (decltype(is_std_optional(symbol_member_or_getter_access(obj, e.name))){}) {
      if (!symbol_member_or_getter_access(obj, e.name).has_value())
        return;
    }

    if (!first) {
      ss << ',';
    }
    first = false;
    if constexpr (has_key(e, s::json_key)) {
      json_encode_value(ss, e.json_key);
    } else
      json_encode_value(ss, symbol_string(e.name));
    ss << ':';

    if constexpr (has_key(e, s::type)) {
      if constexpr (decltype(json_is_vector(e.type)){} or decltype(json_is_object(e.type)){}) {
        return json_encode(ss, symbol_member_or_getter_access(obj, e.name), e.type);
      } else
        json_encode_value(ss, symbol_member_or_getter_access(obj, e.name));
    } else
      json_encode_value(ss, symbol_member_or_getter_access(obj, e.name));
  };

  tuple_map(schema.schema, encode_one_entity);
  ss << '}';
}
} // namespace impl

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_ENCODER


namespace li {

template <typename T> struct json_object_base {

public:
  inline auto downcast() const { return static_cast<const T*>(this); }

  template <typename C, typename O> void encode(C& output, O&& obj) const {
    return impl::json_encode(output, std::forward<O>(obj), *downcast());
  }

  template <typename C, typename... M> void encode(C& output, const metamap<M...>& obj) const {
    return impl::json_encode(output, obj, *downcast());
  }

  template <typename O> std::string encode(O obj) const {
    std::ostringstream ss;
    impl::json_encode(ss, std::forward<O>(obj), *downcast());
    return ss.str();
  }

  template <typename... M> std::string encode(const metamap<M...>& obj) const {
    std::ostringstream ss;
    impl::json_encode(ss, obj, *downcast());
    return ss.str();
  }

  template <typename C, typename O> json_error decode(C& input, O& obj) const {
    return impl::json_decode(input, obj, *downcast());
  }

  template <typename C, typename... M> auto decode(C& input) const {
    auto map = impl::json_object_to_metamap(*downcast());
    impl::json_decode(input, map, *downcast());
    return map;
  }
};

template <typename T> struct json_object_ : public json_object_base<json_object_<T>> {
  json_object_() = default;
  json_object_(const T& s) : schema(s) {}
  T schema;
};

template <typename... S> auto json_object(S&&... s) {
  auto members = std::make_tuple(impl::make_json_object_member(std::forward<S>(s))...);
  return json_object_<decltype(members)>{members};
}

template <typename V> struct json_value_ : public json_object_base<json_value_<V>> {
  json_value_() = default;
};

template <typename V> auto json_value(V&& v) { return json_value_<V>{}; }

template <typename T> struct json_vector_ : public json_object_base<json_vector_<T>> {
  json_vector_() = default;
  json_vector_(const T& s) : schema(s) {}
  T schema;
};

template <typename... S> auto json_vector(S&&... s) {
  auto obj = json_object(std::forward<S>(s)...);
  return json_vector_<decltype(obj)>{obj};
}

template <typename... T> struct json_tuple_ : public json_object_base<json_tuple_<T...>> {
  json_tuple_() = default;
  json_tuple_(const T&... s) : elements(s...) {}
  std::tuple<T...> elements;
};

template <typename... S> auto json_tuple(S&&... s) { return json_tuple_<S...>{s...}; }

struct json_key {
  inline json_key(const char* c) : key(c) {}
  const char* key;
};

template <typename C, typename M> decltype(auto) json_decode(C& input, M& obj) {
  return impl::to_json_schema(obj).decode(input, obj);
}

template <typename C, typename M> decltype(auto) json_encode(C& output, const M& obj) {
  impl::to_json_schema(obj).encode(output, obj);
}

template <typename M> auto json_encode(const M& obj) {
  return std::move(impl::to_json_schema(obj).encode(obj));
}

template <typename A, typename B, typename... C>
auto json_encode(const assign_exp<A, B>& exp, C... c) {
  auto obj = mmm(exp, c...);
  return impl::to_json_schema(obj).encode(obj);
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_JSON


#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_API
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_API

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_DYNAMIC_ROUTING_TABLE
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_DYNAMIC_ROUTING_TABLE


namespace li {

namespace internal {



template <typename V> struct drt_node {

  drt_node() : v_{0, nullptr} {}
  struct iterator {
    const drt_node<V>* ptr;
    std::string_view first;
    V second;

    auto operator-> () { return this; }
    bool operator==(const iterator& b) const { return this->ptr == b.ptr; }
    bool operator!=(const iterator& b) const { return this->ptr != b.ptr; }
  };


  auto end() const { return iterator{nullptr, std::string_view(), V()}; }

  auto& find_or_create(std::string_view r, unsigned int c) {
    if (c == r.size())
      return v_;

    if (r[c] == '/')
      c++; // skip the /
    int s = c;
    while (c < r.size() and r[c] != '/')
      c++;
    std::string_view k = r.substr(s, c - s);

    auto it = children_.find(k);
    if (it != children_.end())
      return children_[k]->find_or_create(r, c);
    else
    {
      auto new_node = new drt_node();
      children_.insert({k, new_node});
      return new_node->find_or_create(r, c);
    }

    return v_;
  }

  template <typename F> void for_all_routes(F f, std::string prefix = "") const {
    if (children_.size() == 0)
      f(prefix, v_);
    else {
      if (prefix.size() && prefix.back() != '/')
        prefix += '/';
      for (auto pair : children_)
        pair.second->for_all_routes(f, prefix + std::string(pair.first));
    }
  }
  iterator find(const std::string_view& r, unsigned int c) const {
    // We found the route r.
    if ((c == r.size() and v_.handler != nullptr) or (children_.size() == 0))
      return iterator{this, r, v_};

    // r does not match any route.
    if (c == r.size() and v_.handler == nullptr)
      return iterator{nullptr, r, v_};

    if (r[c] == '/')
      c++; // skip the first /

    // Find the next /.
    int s = c;
    while (c < r.size() and r[c] != '/')
      c++;

    // k is the string between the 2 /.
    std::string_view k(&r[s], c - s);

    // look for k in the children.
    auto it = children_.find(k);
    if (it != children_.end()) {
      auto it2 = it->second->find(r, c); // search in the corresponding child.
      if (it2 != it->second->end())
        return it2;
    }

    {
      // if one child is a url param {{param_name}}, choose it
      for (auto& kv : children_) {
        auto name = kv.first;
        if (name.size() > 4 and name[0] == '{' and name[1] == '{' and
            name[name.size() - 2] == '}' and name[name.size() - 1] == '}')
          return kv.second->find(r, c);
      }
      return end();
    }
  }

  V v_;
  std::unordered_map<std::string_view, drt_node*> children_;
};
} // namespace internal

template <typename V> struct dynamic_routing_table {

  // Find a route and return reference to a procedure.
  auto& operator[](const std::string_view& r) {
    strings.push_back(std::shared_ptr<std::string>(new std::string(r)));
    std::string_view r2(*strings.back());
    return root.find_or_create(r2, 0);
  }
  auto& operator[](const std::string& r) {
    strings.push_back(std::shared_ptr<std::string>(new std::string(r)));
    std::string_view r2(*strings.back());
    return root.find_or_create(r2, 0);
  }

  // Find a route and return an iterator.
  auto find(const std::string_view& r) const { return root.find(r, 0); }

  template <typename F> void for_all_routes(F f) const { root.for_all_routes(f); }
  auto end() const { return root.end(); }

  std::vector<std::shared_ptr<std::string>> strings;
  internal::drt_node<V> root;
};

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_DYNAMIC_ROUTING_TABLE

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_ERROR
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_ERROR


namespace li {

template <typename E> inline void format_error_(E&) {}

template <typename E, typename T1, typename... T>
inline void format_error_(E& err, T1&& a, T&&... args) {
  err << a;
  format_error_(err, std::forward<T>(args)...);
}

template <typename... T> inline std::string format_error(T&&... args) {
  std::ostringstream ss;
  format_error_(ss, std::forward<T>(args)...);
  return ss.str();
}

struct http_error {
public:
  http_error(int status, const std::string& what) : status_(status), what_(what) {}
  http_error(int status, const char* what) : status_(status), what_(what) {}
  int status() const { return status_; }
  const std::string& what() const { return what_; }

#define LI_HTTP_ERROR(CODE, ERR)                                                                   \
  template <typename... A> static auto ERR(A&&... m) {                                             \
    return http_error(CODE, format_error(m...));                                                   \
  }                                                                                                \
  static auto ERR(const char* w) { return http_error(CODE, w); }

  LI_HTTP_ERROR(400, bad_request)
  LI_HTTP_ERROR(401, unauthorized)
  LI_HTTP_ERROR(403, forbidden)
  LI_HTTP_ERROR(404, not_found)

  LI_HTTP_ERROR(500, internal_server_error)
  LI_HTTP_ERROR(501, not_implemented)

#undef LI_HTTP_ERROR

private:
  int status_;
  std::string what_;
};

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_ERROR


namespace li {

template <typename T, typename F> struct delayed_assignator {
  delayed_assignator(T& t, F f = [](T& t, auto u) { t = u; }) : t(t), f(f) {}

  template <typename U> auto operator=(U&& u) { return f(t, u); }

  T& t;
  F& f;
};

template <typename Req, typename Resp> struct api {

  enum { ANY, GET, POST, PUT, HTTP_DELETE };

  typedef api<Req, Resp> self;

  using H = std::function<void(Req&, Resp&)>;
  struct VH {
    int verb = ANY;
    H handler;
    std::string url_spec;
  };

  H& operator()(std::string_view& route) {
    auto& vh = routes_map_[route];
    vh.verb = ANY;
    vh.url_spec = route;
    return vh.handler;
  }

  H& operator()(int verb, std::string_view r) {
    auto& vh = routes_map_[r];
    vh.verb = verb;
    vh.url_spec = r;
    return vh.handler;
  }
  H& get(std::string_view r) { return this->operator()(GET, r); }
  H& post(std::string_view r) { return this->operator()(POST, r); }
  H& put(std::string_view r) { return this->operator()(PUT, r); }
  H& delete_(std::string_view r) { return this->operator()(HTTP_DELETE, r); }

  int parse_verb(std::string_view method) const {
    if (method == "GET")
      return GET;
    if (method == "PUT")
      return PUT;
    if (method == "POST")
      return POST;
    if (method == "HTTP_DELETE")
      return HTTP_DELETE;
    return ANY;
  }

  void add_subapi(std::string prefix, const self& subapi) {
    subapi.routes_map_.for_all_routes([this, prefix](auto r, VH h) {
      if (!r.empty() && r.back() == '/')
        h.url_spec = prefix + r;
      else
        h.url_spec = prefix + '/' + r;

      this->routes_map_[h.url_spec] = h;
    });
  }

  void print_routes() {
    routes_map_.for_all_routes([this](auto r, auto h) { std::cout << r << '\n'; });
    std::cout << std::endl;
  }
  auto call(std::string_view method, std::string_view route, Req& request, Resp& response) const {
    if (route == last_called_route_)
    {
      if (last_handler_.verb == ANY or parse_verb(method) == last_handler_.verb) {
        request.url_spec = last_handler_.url_spec;
        last_handler_.handler(request, response);
        return;
      } else
        throw http_error::not_found("Method ", method, " not implemented on route ", route);
    }

    // skip the last / of the url.
    std::string_view route2(route);
    if (route2.size() != 0 and route2[route2.size() - 1] == '/')
      route2 = route2.substr(0, route2.size() - 1);

    auto it = routes_map_.find(route2);
    if (it != routes_map_.end()) {
      if (it->second.verb == ANY or parse_verb(method) == it->second.verb) {
        const_cast<self*>(this)->last_called_route_ = route;
        const_cast<self*>(this)->last_handler_ = it->second;
        request.url_spec = it->second.url_spec;
        it->second.handler(request, response);
      } else
        throw http_error::not_found("Method ", method, " not implemented on route ", route2);
    } else
      throw http_error::not_found("Route ", route2, " does not exist.");
  }

  dynamic_routing_table<VH> routes_map_;
  std::string last_called_route_;
  VH last_handler_;
};

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_API

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_HTTP_LISTEN2
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_HTTP_LISTEN2



#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_TCP_SERVER
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_TCP_SERVER





namespace li {

namespace impl {

// Helper to create a TCP/UDP server socket.
static int create_and_bind(int port, int socktype) {
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int s, sfd;

  char port_str[20];
  snprintf(port_str, sizeof(port_str), "%d", port);
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;  /* Return IPv4 and IPv6 choices */
  hints.ai_socktype = socktype; /* We want a TCP socket */
  hints.ai_flags = AI_PASSIVE;  /* All interfaces */

  s = getaddrinfo(NULL, port_str, &hints, &result);
  if (s != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    return -1;
  }

  for (rp = result; rp != NULL; rp = rp->ai_next) {
    sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sfd == -1)
      continue;

    s = bind(sfd, rp->ai_addr, rp->ai_addrlen);
    if (s == 0) {
      /* We managed to bind successfully! */
      break;
    }

    close(sfd);
  }

  if (rp == NULL) {
    fprintf(stderr, "Could not bind: %s\n", strerror(errno));
    return -1;
  }

  freeaddrinfo(result);

  int flags = fcntl(sfd, F_GETFL, 0);
  fcntl(sfd, F_SETFL, flags | O_NONBLOCK);
  ::listen(sfd, SOMAXCONN);

  return sfd;
}

} // namespace impl

static volatile int quit_signal_catched = 0;

struct async_fiber_context;

// Epoll based Reactor:
// Orchestrates a set of fiber (boost::context::continuation).

struct fiber_exception {

  std::string what;
  boost::context::continuation c;
  fiber_exception(fiber_exception&& e) : what{std::move(e.what)}, c{std::move(e.c)} {}
  fiber_exception(boost::context::continuation&& c_, std::string const& what)
      : what{what}, c{std::move(c_)} {}
};

struct async_reactor;

// The fiber context passed to all fibers so they can do
//  yield, non blocking read/write on the socket fd, and subscribe to
//  other file descriptors events.
struct async_fiber_context {

  typedef fiber_exception exception_type;

  async_reactor* reactor;
  boost::context::continuation sink;
  int continuation_idx;
  int socket_fd;

  void epoll_add(int fd, int flags);
  void epoll_mod(int fd, int flags);

  void yield() { sink = sink.resume(); }

  int read(char* buf, int max_size) {
    ssize_t count = ::recv(socket_fd, buf, max_size, 0);
    while (count <= 0) {
      if ((count < 0 and errno != EAGAIN) or count == 0)
        return ssize_t(0);
      sink = sink.resume();
      count = ::recv(socket_fd, buf, max_size, 0);
    }
    return count;
  };

  bool write(const char* buf, int size) {
    if (!buf or !size) {
      // std::cout << "pause" << std::endl;
      sink = sink.resume();
      return true;
    }
    const char* end = buf + size;
    ssize_t count = ::send(socket_fd, buf, end - buf, MSG_NOSIGNAL);
    if (count > 0)
      buf += count;
    while (buf != end) {
      if ((count < 0 and errno != EAGAIN) or count == 0)
        return false;
      sink = sink.resume();
      count = ::send(socket_fd, buf, end - buf, MSG_NOSIGNAL);
      if (count > 0)
        buf += count;
    }
    return true;
  };
};

struct async_reactor {

  typedef boost::context::continuation continuation;

  int epoll_fd;
  std::vector<continuation> fibers;
  std::vector<int> fd_to_fiber_idx;

  continuation& fd_to_fiber(int fd) {
    assert(fd >= 0 and fd < fd_to_fiber_idx.size());
    int fiber_idx = fd_to_fiber_idx[fd];
    assert(fiber_idx >= 0 and fiber_idx < fibers.size());
    
    return fibers[fiber_idx];
  }

  void epoll_ctl(int epoll_fd, int fd, int action, uint32_t flags) {
    epoll_event event;
    memset(&event, 0, sizeof(event));
    event.data.fd = fd;
    event.events = flags;
    if (-1 == ::epoll_ctl(epoll_fd, action, fd, &event) and errno != EEXIST)
      std::cout << "epoll_ctl error: " <<  strerror(errno) << std::endl;    
  };

  void epoll_add(int new_fd, int flags, int fiber_idx = -1) {
    epoll_ctl(epoll_fd, new_fd, EPOLL_CTL_ADD, flags);
    // Associate new_fd to the fiber.
    if (int(fd_to_fiber_idx.size()) < new_fd + 1)
      fd_to_fiber_idx.resize((new_fd + 1) * 2, -1);
    fd_to_fiber_idx[new_fd] = fiber_idx;
  }

  void epoll_mod(int fd, int flags) { epoll_ctl(epoll_fd, fd, EPOLL_CTL_MOD, flags); }

  template <typename H> void event_loop(int listen_fd, H handler) {

    this->epoll_fd = epoll_create1(0);
    epoll_ctl(epoll_fd, listen_fd, EPOLL_CTL_ADD, EPOLLIN | EPOLLET);

    const int MAXEVENTS = 64;
    epoll_event events[MAXEVENTS];

    // Main loop.
    while (!quit_signal_catched) {

      int n_events = epoll_wait(epoll_fd, events, MAXEVENTS, 1);
      if (quit_signal_catched)
        break;

      if (n_events == 0)
        for (int i = 0; i < fibers.size(); i++)
          if (fibers[i])
            fibers[i] = fibers[i].resume();

      for (int i = 0; i < n_events; i++) {

        int event_flags = events[i].events;
        int event_fd = events[i].data.fd;

        // Handle error on sockets.
          if (event_flags & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
          if (event_fd == listen_fd) {
            std::cout << "FATAL ERROR: Error on server socket " << event_fd << std::endl;
            quit_signal_catched = true;
          } else {
            continuation& fiber = fd_to_fiber(event_fd);
            if (fiber)
              fiber = fiber.resume_with(std::move([](auto&& sink) {
                throw fiber_exception(std::move(sink), "EPOLLRDHUP");
                return std::move(sink);
              }));
          }
        }
        // Handle new connections.
        else if (listen_fd == event_fd) {
          while (true) {

            // ============================================
            // ACCEPT INCOMMING CONNECTION
            struct sockaddr in_addr;
            socklen_t in_len;
            int socket_fd;
            in_len = sizeof in_addr;
            socket_fd = accept(listen_fd, &in_addr, &in_len);
            if (socket_fd == -1)
              break;
            // ============================================

            // ============================================
            // Find a free fiber for this new connection.
            int fiber_idx = 0;
            while (fiber_idx < fibers.size() && fibers[fiber_idx])
              fiber_idx++;
            if (fiber_idx >= fibers.size())
              fibers.resize((fibers.size() + 1) * 2);
            assert(fiber_idx < fibers.size());
            // ============================================

            // ============================================
            // Subscribe epoll to the socket file descriptor.
            if (-1 == fcntl(socket_fd, F_SETFL, fcntl(socket_fd, F_GETFL, 0) | O_NONBLOCK))
              continue;
            this->epoll_add(socket_fd, EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET, fiber_idx);
            // ============================================

            // ============================================
            // Simply utility to close fd at the end of a scope.
            struct scoped_fd {
              int fd;
              ~scoped_fd() {
                if (0 != close(fd))
                  std::cerr << "Error when closing file descriptor " << fd << ": "
                            << strerror(errno) << std::endl;
              }
            };
            // ============================================

            // =============================================
            // Spawn a new continuation to handle the connection.
            fibers[fiber_idx] =
                boost::context::callcc([this, socket_fd, fiber_idx, &handler](continuation&& sink) {
                  scoped_fd sfd{socket_fd}; // Will finally close the fd.
                  auto ctx = async_fiber_context{this, std::move(sink), fiber_idx, socket_fd};
                  try {
                    handler(ctx);
                  } catch (fiber_exception& ex) {
                    return std::move(ex.c);
                  } catch (const std::runtime_error& e) {
                    std::cerr << "FATAL ERRROR: exception in fiber: " << e.what() << std::endl;
                    assert(0);
                    return std::move(sink);
                  }
                  return std::move(ctx.sink);
                });
            // =============================================
          }
        } else // Data available on existing sockets. Wake up the fiber associated with
               // event_fd.
        {
          if (event_fd >= 0 && event_fd < fd_to_fiber_idx.size()) {
            auto& fiber = fd_to_fiber(event_fd);
            if (fiber)
              fiber = fiber.resume();
          } else
            std::cerr << "Epoll returned a file descriptor that we did not register: " << event_fd
                      << std::endl;
        }
      }
    }
    std::cout << "END OF EVENT LOOP" << std::endl;
    close(epoll_fd);
  }
};

static void shutdown_handler(int sig) {
  quit_signal_catched = 1;
  std::cout << "The server will shutdown..." << std::endl;
}

void async_fiber_context::epoll_add(int fd, int flags) {
  reactor->epoll_add(fd, flags, continuation_idx);
}
void async_fiber_context::epoll_mod(int fd, int flags) { reactor->epoll_mod(fd, flags); }

template <typename H> void start_tcp_server(int port, int socktype, int nthreads, H conn_handler) {

  struct sigaction act;
  memset(&act, 0, sizeof(act));
  act.sa_handler = shutdown_handler;

  sigaction(SIGINT, &act, 0);
  sigaction(SIGTERM, &act, 0);
  sigaction(SIGQUIT, &act, 0);

  int server_fd = impl::create_and_bind(port, socktype);
  std::vector<std::thread> ths;
  for (int i = 0; i < nthreads; i++)
    ths.push_back(std::thread([&] {
      async_reactor reactor;
      reactor.event_loop(server_fd, conn_handler);
    }));

  for (auto& t : ths)
    t.join();

  close(server_fd);
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_TCP_SERVER

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_URL_UNESCAPE
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_URL_UNESCAPE


namespace li {
std::string_view url_unescape(std::string_view str) {
  char* o = (char*)str.data();
  char* c = (char*)str.data();
  const char* end = c + str.size();

  while (c < end) {
    if (*c == '%' && c < end - 2) {
      char number[3];
      number[0] = c[1];
      number[1] = c[2];
      number[2] = 0;
      *o = char(strtol(number, nullptr, 16));
      c += 2;
    } else if (o != c)
      *o = *c;
    o++;
    c++;
  }
  return std::string_view(str.data(), o - str.data());
}
} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_URL_UNESCAPE


namespace li {  


struct output_buffer
{

  output_buffer() 
  : flush_([] (const char*, int) {})
  {
  }

  output_buffer(output_buffer&& o)
    : buffer_(o.buffer_),
      own_buffer_(o.own_buffer_),
      cursor_(o.cursor_),
      end_(o.end_),
      flush_(o.flush_)
  {
    o.buffer_ = nullptr;
    o.own_buffer_ = false;
  }

  output_buffer(int capacity, 
                std::function<void(const char*, int)> flush_ = [] (const char*, int) {})
    : buffer_(new char[capacity]),
      own_buffer_(true),
      cursor_(buffer_),
      end_(buffer_ + capacity),
      flush_(flush_)
  {
    assert(buffer_);
  }

  output_buffer(void* buffer, int capacity, 
                std::function<void(const char*, int)> flush_ = [] (const char*, int) {})
    : buffer_((char*)buffer),
      own_buffer_(false),
      cursor_(buffer_),
      end_(buffer_ + capacity),
      flush_(flush_)
  {
    assert(buffer_);
  }

  ~output_buffer() {
    if (own_buffer_)
      delete[] buffer_;
  }
  
  output_buffer& operator=(output_buffer&& o)
  {
    buffer_ = o.buffer_;
    own_buffer_ = o.own_buffer_;
    cursor_ = o.cursor_;
    end_ = o.end_;
    flush_ = o.flush_;
    o.buffer_ = nullptr;
    o.own_buffer_ = false;
    return *this;
  }

  void reset() 
  {
    cursor_ = buffer_;
  }

  std::size_t size()
  {
    return cursor_ - buffer_;
  }
  void flush()
  {
    flush_(buffer_, size());
    reset();
  }

  output_buffer& operator<<(std::string_view s)
  {
    if (cursor_ + s.size() >= end_)
      flush();

    assert(cursor_ + s.size() < end_);
    memcpy(cursor_, s.data(), s.size());
    cursor_ += s.size();
    return *this;
  }

  output_buffer& operator<<(const char* s)
  {
    return operator<<(std::string_view(s, strlen(s)));   
  }
  output_buffer& operator<<(char v)
  {
    cursor_[0] = v;
    cursor_++;
    return *this;
  }

  output_buffer& operator<<(std::size_t v)
  {
    if (v == 0) operator<<('0');

    char buffer[10];
    char* str_start = buffer;
    for (int i = 0; i < 10; i++)
    {
      if (v > 0) str_start = buffer + 9 - i;
      buffer[9 - i] = (v % 10) + '0';
      v /= 10;
    }
    operator<<(std::string_view(str_start, buffer + 10 - str_start));
    return *this;
  }
  // template <typename I>
  // output_buffer& operator<<(unsigned long s)
  // {
  //   typedef std::array<char, 150> buf_t;
  //   buf_t b = boost::lexical_cast<buf_t>(v);
  //   return operator<<(std::string_view(b.begin(), strlen(b.begin())));
  // }


  template <typename I>
  output_buffer& operator<<(I v)
  {
    typedef std::array<char, 150> buf_t;
    buf_t b = boost::lexical_cast<buf_t>(v);
    return operator<<(std::string_view(b.begin(), strlen(b.begin())));
  }
  
  std::string_view to_string_view() { return std::string_view(buffer_, cursor_ - buffer_); }

  char* buffer_;
  bool own_buffer_;
  char* cursor_;
  char* end_;
  std::function<void(const char*s, int d)> flush_;
};


namespace http_async_impl {  

char* date_buf = nullptr;
int date_buf_size = 0;

thread_local std::unordered_map<std::string, std::string_view> static_files;

struct read_buffer
{

  //std::array<char, 20*1024> buffer_;
  std::vector<char> buffer_;
  int cursor = 0; // First index of the currently used buffer area
  int end = 0; // Index of the last read character
  
  read_buffer()
    : buffer_(5 * 1024),
      cursor(0),
      end(0)
  {}

  // Free unused space in the buffer in [i1, i2[.
  // This may move data in [i2, end[ if needed.
  // Return the
  void free(int i1, int i2)
  {
    assert(i1 < i2);
    assert(i1 >= 0 and i1 < buffer_.size());
    assert(i2 > 0 and i2 <= buffer_.size());

    if (i1 == cursor and i2 == end) // eat the whole buffer.
      cursor = end = 0;
    else if (i1 == cursor) // eat the beggining of the buffer.
      cursor = i2;
    else if (i2 == end) end = i1; // eat the end of the buffer.
    else if (i2 != end) // eat somewhere in the middle.
    {
      if (buffer_.size() - end < buffer_.size() / 4)
      {
        if (end - i2 > i2 - i1) // use memmove if overlap.
          std::memmove(buffer_.data() + i1, buffer_.data() + i2, end - i2);
        else
          std::memcpy(buffer_.data() + i1, buffer_.data() + cursor, end - cursor);
      }
    }
  }

  void free(const char* i1, const char* i2) { 
    assert(i1 >= buffer_.data());
    assert(i1 <= &buffer_.back());
    //std::cout << (i2 - &buffer_.front()) << " " << buffer_.size() <<  << std::endl;
    assert(i2 >= buffer_.data() and i2 <= &buffer_.back() + 1);
    free(i1 - buffer_.data(), i2 - buffer_.data()); 
  }
  void free(const std::string_view& str) { free(str.data(), str.data() + str.size()); }
  
  // private: Read more data
  // Read from ptr until character x.
  // read n more characters at address ptr.
  // eat n first/last character.
  // free part of the buffer and more data if needed.

  // Read more data.
  // Return 0 on error.
  template <typename F>
  int read_more(F& fiber, int size = -1)
  {
    if (int(buffer_.size()) <= end - 100)
    {
      // reset buffer_.
      cursor = 0; end = 0;
      // if (buffer_.size() > (10*1024*1024))
      //   return 0; // Buffer is full. Error and return.
      // else 
      {
        // std::cout << "RESIZE" << std::endl;
        // buffer_.resize(buffer_.size() * 2);
      }
    }

    if (size == -1) size = buffer_.size() - end;
    int received = fiber.read(buffer_.data() + end, size);

    if (received == 0) return 0; // Socket closed, return.
    end = end + received;
    if (end == buffer_.size())
    {
      //std::cerr << "Request too long." << std::endl;
      throw std::runtime_error("Request too long.");
    }
    //std::cout << "read size " << end << std::endl;
    return received;
  }
  template <typename F>
  std::string_view read_more_str(F& fiber)
  {
    int l = read_more(fiber);
    return std::string_view(buffer_.data() + end - l);
  }

  template <typename F>
  std::string_view read_n(F&& fiber, const char* start, int size)
  {
    int str_start = start - buffer_.data();
    int str_end = size + str_start;
    if (end < str_end)
    {
      // Read more body on the socket.
      int current_size = end - str_start;
      while (current_size < size)
          current_size += read_more(fiber);
    }
    return std::string_view(start, size);
  }
  
  template <typename F>
  std::string_view read_until(F&& fiber, const char*& start, char delimiter)
  {
    const char* str_end = start;

    while (true)
    {
      const char* buffer_end = buffer_.data() + end;
      while (str_end < buffer_end and *str_end != delimiter) str_end++;

      if (*str_end == delimiter) break;
      else {
        if (!read_more(fiber)) break;
      }
    }

    auto res = std::string_view(start, str_end - start);
    start = str_end + 1;
    return res;
  }

  bool empty() const { return cursor == end; }

  // Return the amount of data currently available to read.
  int current_size() { return end - cursor; }

  // Reset the buffer. Copy remaining data at the beggining if there is some.
  void reset() {
    assert(cursor <= end);
    if (cursor == end)
      end = cursor = 0;
    else
    {
      if (cursor > end - cursor) // use memmove if overlap.
        std::memmove(buffer_.data(), buffer_.data() + cursor, end - cursor);
      else
        std::memcpy(buffer_.data(), buffer_.data() + cursor, end - cursor);

      // if (
      // memcpy(buffer_.data(), buffer_.data() + cursor, end - cursor);

      end = end - cursor;
      cursor = 0;
    }

  }

  // On success return the number of bytes read.
  // On error return 0.
  char* data() { return buffer_.data(); }
};


struct http_ctx {

  http_ctx(read_buffer& _rb,
           async_fiber_context& _fiber)
    : rb(_rb),
      fiber(_fiber)
  {
    get_parameters_map.reserve(10);
    response_headers.reserve(20);

    output_stream = output_buffer(50 * 1024, 
                                  [&] (const char* d, int s) { 
                                        fiber.write(d, s); 
                                    });

    headers_stream = output_buffer(1000,
                                  [&] (const char* d,int s) { output_stream << std::string_view(d, s); });

    json_stream = output_buffer(50 * 1024,
                                [&] (const char* d,int s) { output_stream << std::string_view(d, s); });

  }

  http_ctx& operator=(const http_ctx&) = delete;
  http_ctx(const http_ctx&) = delete;

  std::string_view header(const char* key)
  {
    if (!header_map.size())
      index_headers();
    return header_map[key];
  }
  
  std::string_view cookie(const char* key)
  {
    if (!cookie_map.size())
      index_cookies();
    return cookie_map[key];
  }

  std::string_view get_parameter(const char* key)
  {
    if (!url_.size())
      parse_first_line();
    return get_parameters_map[key];
  }
  
  // std::string_view post_parameter(const char* key)
  // {
  //   if (!is_body_read_)
  //   {
  //     //read_whole_body();
  //     parse_post_parameters();
  //   }
  //   return post_parameters_map[key];
  // }
 
  // std::string_view read_body(char* buf, int buf_size)
  // {
  //   // Try to read Content-Length

  //   // if part of the body is in the request buffer
  //   //  return it and mark it as read.
  //   // otherwise call read on the socket.

  //   // handle chunked encoding here if needed.
  // }

  // void sendfile(std::string path)
  // {
  //   char buffer[10200];
  //   //std::cout << uint64_t(output_stream.buffer_) << " " << uint64_t(output_buffer_space) << std::endl;
  //   output_buffer output_stream(buffer, sizeof(buffer));

  //   struct stat stat_buf;
  //   int read_fd = open (path.c_str(), O_RDONLY);
  //   fstat (read_fd, &stat_buf);


  //   format_top_headers(output_stream);
  //   output_stream << headers_stream.to_string_view();
  //   output_stream << "Content-Length: " << size_t(stat_buf.st_size) << "\r\n\r\n"; //Add body
  //   auto m = output_stream.to_string_view();
  //   write(m.data(), m.size());
    
  //   off_t offset = 0;
  //   while (true)
  //   {
  //     int ret = ::sendfile(socket_fd, read_fd, &offset, stat_buf.st_size);
  //     if (ret == EAGAIN)
  //       write(nullptr, 0);
  //     else break;
  //   }
  // }

  std::string_view url()
  {
    if (!url_.size())
      parse_first_line();
    return url_;
  }
  std::string_view method()
  {
    if (!method_.size())
      parse_first_line();
    return method_;
  }
  std::string_view http_version()
  {
    if (!url_.size())
      parse_first_line();
    return http_version_;
  }

  inline void format_top_headers(output_buffer& output_stream)
  {
    output_stream << "HTTP/1.1 " << status_ << "\r\n";
    output_stream << "Date: " << std::string_view(date_buf, date_buf_size) << "\r\n";
    output_stream << "Connection: keep-alive\r\nServer: Lithium\r\n";
  }
  
  void prepare_request()
  {
    //parse_first_line();
    response_headers.clear();
    content_length_ = 0;
    chunked_ = 0;
    
    for (int i = 1; i < header_lines_size - 1; i++)
    {
      const char* line_end = header_lines[i + 1]; // last line is just an empty line.
      const char* cur = header_lines[i];

      if (*cur != 'C' and *cur != 'c') continue;

      std::string_view key = split(cur, line_end, ':');

      auto get_value = [&] {
        std::string_view value = split(cur, line_end, '\r');
        while (value[0] == ' ')
          value = std::string_view(value.data() + 1, value.size() - 1);
        return value;
      };

      if (key == "Content-Length")
        content_length_ = atoi(get_value().data());
      else if (key == "Content-Type")
      {
        content_type_ = get_value();
        chunked_ = (content_type_ == "chunked");
      }
      
      // header_map[key] = value;
      // std::cout << key << " -> " << value << std::endl;
    }

  }

  //void respond(std::string s) {return respond(std::string_view(s)); }

  // void respond(const char* s)
  // {
  //   return respond(std::string_view(s, strlen(s)));
  // }

  void respond(const std::string_view& s)
  {
    response_written_ = true;
    format_top_headers(output_stream);
    headers_stream.flush(); // flushes to output_stream.
    output_stream << "Content-Length: " << s.size() << "\r\n\r\n" << s; //Add body
  }

  template <typename O>
  void respond_json(const O& obj)
  {
    response_written_ = true;
    json_stream.reset();
    json_encode(json_stream, obj);

    format_top_headers(output_stream);
    headers_stream.flush(); // flushes to output_stream.
    output_stream << "Content-Length: " << json_stream.to_string_view().size() << "\r\n\r\n";
    json_stream.flush(); // flushes to output_stream.
  }
  

  void respond_if_needed()
  {
    if (!response_written_)
    {
      response_written_ = true;

      format_top_headers(output_stream);
      output_stream << headers_stream.to_string_view();
      output_stream << "Content-Length: 0\r\n\r\n"; //Add body
    }
  }

  
  void set_header(std::string_view k, std::string_view v) { 
    headers_stream << k << ": " << v << "\r\n"; 
  }

  void set_cookie(std::string_view k, std::string_view v) { 
    headers_stream << "Set-Cookie: " << k << '=' << v << "\r\n";
  }

  void set_status(int status) {

    switch (status)
    {
    case 200: status_ = "200 OK"; break;
    case 201: status_ = "201 Created"; break;
    case 204: status_ = "204 No Content"; break;
    case 304: status_ = "304 Not Modified"; break;
    case 400: status_ = "400 Bad Request"; break;
    case 401: status_ = "401 Unauthorized"; break;
    case 402: status_ = "402 Not Found"; break;
    case 403: status_ = "403 Forbidden"; break;
    case 404: status_ = "404 Not Found"; break;
    case 409: status_ = "409 Conflict"; break;
    case 500: status_ = "500 Internal Server Error"; break;
    default: status_ = "200 OK"; break;
    }
    
  }
  
  void send_static_file(const char* path)
  {
    auto it = static_files.find(path);
    if (static_files.end() == it or !it->second.size())
    {
      int fd = open(path, O_RDONLY);
      if (fd == -1)
        throw http_error::not_found("File not found.");
      int file_size = lseek(fd, (size_t)0, SEEK_END);
      auto content = std::string_view((char*)mmap(0, file_size, PROT_READ, MAP_SHARED, fd, 0), file_size);
      static_files.insert({path, content});
      respond(content);
    }
    else
      respond(it->second);
  }
  
  //private:

  void add_header_line(const char* l) { header_lines[header_lines_size++] = l; }
  const char* last_header_line() { return header_lines[header_lines_size - 1]; }

  // split a string, starting from cur and ending with split_char.
  // Advance cur to the end of the split.
  std::string_view split(const char*& cur,
                         const char* line_end, char split_char)
  {

    const char* start = cur;
    while (start < (line_end-1) and *start == split_char) start++;
    const char* end = start + 1;
    while (end < (line_end-1) and *end != split_char) end++;
    cur = end + 1;
    if (*end == split_char)
      return std::string_view(start, cur - start - 1);
    else
      return std::string_view(start, cur - start);
  }
  
  void index_headers()
  {
    for (int i = 1; i < header_lines_size - 1; i++)
    {
      const char* line_end = header_lines[i + 1]; // last line is just an empty line.
      const char* cur = header_lines[i];

      std::string_view key = split(cur, line_end, ':');
      std::string_view value = split(cur, line_end, '\r');
      while (value[0] == ' ')
        value = std::string_view(value.data() + 1, value.size() - 1);
      header_map[key] = value;
      //std::cout << key << " -> " << value << std::endl;
    }
  }

  void index_cookies()
  {
    std::string_view cookies = header("Cookie");
    if (!cookies.data()) return;
    const char* line_end = &cookies.back() + 1;
    const char* cur = &cookies.front();
    while(cur < line_end)
    {

      std::string_view key = split(cur, line_end, '=');
      std::string_view value = split(cur, line_end, ';');
      while (key[0] == ' ')
        key = std::string_view(key.data() + 1, key.size() - 1);
      cookie_map[key] = value;
    }
  }

  template <typename C>
  void url_decode_parameters(std::string_view content, C kv_callback)
  {
    const char* c = content.data();
    const char* end = c + content.size();
    if (c < end)
    {
      while (c < end)
      {
        std::string_view key = split(c, end, '=');
        std::string_view value = split(c, end, '&');
        kv_callback(key, value);
        // printf("kv: '%.*s' -> '%.*s'\n", key.size(), key.data(), value.size(), value.data());
      }
    }    
  }
    
  void parse_first_line()
  {    
    const char* c = header_lines[0];
    const char* end = header_lines[1];

    method_ = split(c, end, ' ');
    url_ = split(c, end, ' ');
    http_version_ = split(c, end, '\r');

    // url get parameters.
    c = url_.data();
    end = c + url_.size();
    url_ = split(c, end, '?');
    get_parameters_string_ = std::string_view(c, end - c);

  }

  std::string_view get_parameters_string()
  {
    if (!get_parameters_string_.data())
      parse_first_line();
    return get_parameters_string_;
  }
  template <typename F>
  void parse_get_parameters(F processor)
  {
    url_decode_parameters(get_parameters_string(), processor);
  }

  template <typename F>
  void read_body(F callback)
  {
    is_body_read_ = true;

    if (!chunked_ and !content_length_)
      body_end_ = body_start.data();
    else if (content_length_)
    {
      std::string_view res;
      int n_body_read = 0;

      // First return part of the body already in memory.
      int l = std::min(int(body_start.size()), content_length_);
      callback(std::string_view(body_start.data(), l));
      n_body_read += l;
      
      while (content_length_ > n_body_read)
      {
        std::string_view part = rb.read_more_str(fiber);
        int l = part.size();
        int bl = std::min(l, content_length_ - n_body_read);
        part = std::string_view(part.data(), bl);
        callback(part);
        rb.free(part);
        body_end_ = part.data();
        n_body_read += part.size();
      }

    }
    else if (chunked_)
    {
      // Chunked decoding.
      const char* cur = body_start.data();
      int chunked_size = strtol(rb.read_until(read, cur, '\r').data(), nullptr, 16);
      cur++; // skip \n
      while (chunked_size > 0)
      {
        // Read chunk.
        std::string_view chunk = rb.read_n(read, cur, chunked_size);
        callback(chunk);
        rb.free(chunk);
        cur += chunked_size + 2; // skip \r\n.

        // Read next chunk size.
        chunked_size = strtol(rb.read_until(read, cur, '\r').data(), nullptr, 16);
        cur++; // skip \n
      }
      cur += 2;// skip the terminaison chunk.
      body_end_ = cur;
      body_ = std::string_view(body_start.data(), cur - body_start.data());
    }
  }
  
  std::string_view read_whole_body()
  {
    if (!chunked_ and !content_length_)
    {
      is_body_read_ = true;
      body_end_ = body_start.data();
      return std::string_view(); // No body.
    }

    if (content_length_)
    {
      body_ = rb.read_n(fiber, body_start.data(), content_length_);
      body_end_ = body_.data() + content_length_;
    }
    else if (chunked_)
    {
      // Chunked decoding.
      char* out = (char*) body_start.data();
      const char* cur = body_start.data();
      int chunked_size = strtol(rb.read_until(fiber, cur, '\r').data(), nullptr, 16);
      cur++; // skip \n
      while (chunked_size > 0)
      {
        // Read chunk.
        std::string_view chunk = rb.read_n(fiber, cur, chunked_size);
        cur += chunked_size + 2; // skip \r\n.
        // Copy the body into a contiguous string.
        if (out + chunk.size() > chunk.data()) // use memmove if overlap.
          std::memmove(out, chunk.data(), chunk.size());
        else
          std::memcpy(out, chunk.data(), chunk.size());
        
        out += chunk.size();

        // Read next chunk size.
        chunked_size = strtol(rb.read_until(fiber, cur, '\r').data(), nullptr, 16);
        cur++; // skip \n
      }
      cur += 2;// skip the terminaison chunk.
      body_end_ = cur;
      body_ = std::string_view(body_start.data(), out - body_start.data());
    }

    is_body_read_ = true;    
    return body_;
  }

  void read_multipart_formdata()
  {

  }

  template <typename F>
  void post_iterate(F kv_callback)
  {
    if (is_body_read_) // already in memory.
      url_decode_parameters(body_, kv_callback);
    else // stream the body.
    {
      std::string_view current_key;

      read_body([] (std::string_view part) {
          // read key if needed.
          //if (!current_key.size())
          // call kv callback if some value is available.          
        });
    }
  }
  
  // Read post parameters in the body.
  std::unordered_map<std::string_view, std::string_view> post_parameters()
  {
    if (content_type_ == "application/x-www-form-urlencoded")
    {
      if (!is_body_read_)
        read_whole_body();
      url_decode_parameters(body_, [&] (auto key, auto value)
                            { post_parameters_map[key] = value; });
      return post_parameters_map;
    }
    else
    {
      // fixme: return bad request here.
    }
    return post_parameters_map;
  }

  void prepare_next_request()
  {
    if (!is_body_read_)
      read_whole_body();

    // std::cout <<"free line0: " << uint64_t(header_lines[0]) << std::endl;
    // std::cout << rb.current_size() << " " << rb.cursor << std::endl;
    rb.free(header_lines[0], body_end_);
    // std::cout << rb.current_size() << " " << rb.cursor << std::endl;
    //rb.cursor = rb.end = 0;
    //assert(rb.cursor == 0);
    headers_stream.reset();
    status_ = "200 OK";
    method_ = std::string_view();
    url_ = std::string_view();
    http_version_ = std::string_view();
    content_type_ = std::string_view();
    header_map.clear();
    cookie_map.clear();
    response_headers.clear();
    get_parameters_map.clear();
    post_parameters_map.clear();
    get_parameters_string_ = std::string_view();
    response_written_ = false;
  }

  void flush_responses() {
    output_stream.flush();
  }

  int socket_fd;
  read_buffer& rb;

  const char* status_ = "200 OK";
  std::string_view method_;
  std::string_view url_;
  std::string_view http_version_;
  std::string_view content_type_;
  bool chunked_;
  int content_length_;
  std::unordered_map<std::string_view, std::string_view> header_map;
  std::unordered_map<std::string_view, std::string_view> cookie_map;
  std::vector<std::pair<std::string_view, std::string_view>> response_headers;
  std::unordered_map<std::string_view, std::string_view> get_parameters_map;
  std::unordered_map<std::string_view, std::string_view> post_parameters_map;
  std::string_view get_parameters_string_;
  //std::vector<std::string> strings_saver;

  bool is_body_read_ = false;
  std::string body_local_buffer_;
  std::string_view body_;
  std::string_view body_start;
  const char* body_end_ = nullptr;
  const char* header_lines[100];
  int header_lines_size = 0;
  async_fiber_context& fiber;

  output_buffer headers_stream;
  bool response_written_ = false;

  output_buffer output_stream;
  output_buffer json_stream;
};  

template <typename F>
auto make_http_processor(F handler)
{
  return [handler] (async_fiber_context& fiber) {

    try {
      read_buffer rb;
      bool socket_is_valid = true;

      http_ctx ctx = http_ctx(rb, fiber);
      ctx.socket_fd = fiber.socket_fd;
      
      while (true)
      {
        ctx.is_body_read_ = false;
        ctx.header_lines_size = 0;
        // Read until there is a complete header.
        int header_start = rb.cursor;
        int header_end = rb.cursor;
        assert(header_start >= 0);
        assert(header_end >= 0);
        assert(ctx.header_lines_size == 0);
        ctx.add_header_line(rb.data() + header_end);
        assert(ctx.header_lines_size == 1);

        bool complete_header = false;
        while (!complete_header)
        {
          // Read more data from the socket.
          if (rb.empty())
            if (!rb.read_more(fiber)) 
              return;

          // Look for end of header and save header lines.
          {
            const char * cur = rb.data() + header_end;
            while ((cur - rb.data()) < rb.end - 3)
            {
              //if (!strncmp(rb.data() + header_end, "\r\n", 2)) // slower 
              if (cur[0] == '\r' and cur[1] == '\n')
              {
                ctx.add_header_line(cur + 2);
                //header_end += 2;
                cur+=2;
                //if ((rb.data() + header_end)[0] == '\r' and (rb.data() + header_end)[1] == '\n')
                if (cur[0] == '\r' and cur[1] == '\n') // Seems to be the fastest.
                //if (!strncmp(rb.data() + header_end, "\r\n", 2))
                {
                  complete_header = true;
                  cur+=2;
                  header_end = cur - rb.data();
                  break;
                }
              }
              else
                cur++;
            }
          }
        }

        // Header is complete. Process it.
        // Run the handler.
        assert(rb.cursor <= rb.end);
        ctx.body_start = std::string_view(rb.data() + header_end, rb.end - header_end);
        ctx.prepare_request();
        handler(ctx);
        assert(rb.cursor <= rb.end);

        // Update the cursor the beginning of the next request.
        ctx.prepare_next_request();
        // if read buffer is empty, we can flush the output buffer.
        if (rb.empty())// || ctx.output_stream.size() > 100000)
          ctx.flush_responses();

      }
    }
    catch (const std::runtime_error& e) 
    {
      std::cerr << "Error: " << e.what() << std::endl;
      return;
    }
  };

}

} // namespace http_async_impl
}


#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_REQUEST
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_REQUEST


#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_URL_DECODE
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_URL_DECODE

#if defined(_MSC_VER)
#endif


namespace li {
// Decode a plain value.
template <typename O>
std::string_view url_decode2(std::set<void*>& found, std::string_view str, O& obj) {
  if (str.size() == 0)
    throw std::runtime_error(format_error("url_decode error: expected key end"));

  if (str[0] != '=')
    throw std::runtime_error(format_error("url_decode error: expected =, got ", str[0]));

  int start = 1;
  int end = 1;

  while (str.size() != end && str[end] != '&')
    end++;

  if (start != end) {
    std::string content(str.substr(start, end - start));
    // Url escape.
    //content.resize(MHD_http_unescape(&content[0]));
    if constexpr(std::is_same<std::remove_reference_t<decltype(obj)>, std::string>::value or
                std::is_same<std::remove_reference_t<decltype(obj)>, std::string_view>::value)
      obj = content;
    else
      obj = boost::lexical_cast<O>(content);
    found.insert(&obj);
  }
  if (end == str.size())
    return str.substr(end, 0);
  else
    return str.substr(end + 1, str.size() - end - 1);
}

template <typename O>
std::string_view url_decode2(std::set<void*>& found, std::string_view str, std::optional<O>& obj) {
  O o;
  auto ret = url_decode2(found, str, o);
  obj = o;
  return ret;
}

template <typename... O>
std::string_view url_decode2(std::set<void*>& found, std::string_view str, metamap<O...>& obj,
                             bool root = false);

// Decode an array element.
template <typename O>
std::string_view url_decode2(std::set<void*>& found, std::string_view str, std::vector<O>& obj) {
  if (str.size() == 0)
    throw std::runtime_error(format_error("url_decode error: expected key end", str[0]));

  if (str[0] != '[')
    throw std::runtime_error(format_error("url_decode error: expected [, got ", str[0]));

  // Get the index substring.
  int index_start = 1;
  int index_end = 1;

  while (str.size() != index_end and str[index_end] != ']')
    index_end++;

  if (str.size() == 0)
    throw std::runtime_error(format_error("url_decode error: expected key end", str[0]));

  auto next_str = str.substr(index_end + 1, str.size() - index_end - 1);

  if (index_end == index_start) // [] syntax, push back a value.
  {
    O x;
    auto ret = url_decode2(found, next_str, x);
    obj.push_back(x);
    return ret;
  } else // [idx] set index idx.
  {
    int idx = std::strtol(str.data() + index_start, nullptr, 10);
    if (idx >= 0 and idx <= 9999) {
      if (int(obj.size()) <= idx)
        obj.resize(idx + 1);
      return url_decode2(found, next_str, obj[idx]);
    } else
      throw std::runtime_error(format_error("url_decode error: out of bound array subscript."));
  }
}

// Decode an object member.
template <typename... O>
std::string_view url_decode2(std::set<void*>& found, std::string_view str, metamap<O...>& obj,
                             bool root) {
  if (str.size() == 0)
    throw http_error::bad_request("url_decode error: expected key end", str[0]);

  int key_start = 0;
  int key_end = key_start + 1;

  int next = 0;

  if (!root) {
    if (str[0] != '[')
      throw std::runtime_error(format_error("url_decode error: expected [, got ", str[0]));

    key_start = 1;
    while (key_end != str.size() && str[key_end] != ']' && str[key_end] != '=')
      key_end++;

    if (key_end == str.size())
      throw std::runtime_error("url_decode error: unexpected end.");
    next = key_end + 1; // skip the ]
  } else {
    while (key_end < str.size() && str[key_end] != '[' && str[key_end] != '=')
      key_end++;
    next = key_end;
  }

  auto key = str.substr(key_start, key_end - key_start);
  auto next_str = str.substr(next, str.size() - next);

  std::string_view ret;
  map(obj, [&](auto k, auto& v) {
    if (li::symbol_string(k) == key) {
      try {
        ret = url_decode2(found, next_str, v);
      } catch (std::exception e) {
        throw std::runtime_error(
            format_error("url_decode error: cannot decode parameter ", li::symbol_string(k)));
      }
    }
  });
  return ret;
}

template <typename O>
std::string url_decode_check_missing_fields(const std::set<void*>& found, std::optional<O>& obj) {
  return std::string();
}

template <typename O>
std::string url_decode_check_missing_fields(const std::set<void*>& found, O& obj) {
  if (found.find(&obj) == found.end())
    return " ";
  else
    return std::string();
}

template <typename O>
std::string url_decode_check_missing_fields(const std::set<void*>& found, std::vector<O>& obj) {
  // Fixme : implement missing fields checking in std::vector<metamap<O...>>
  // for (int i = 0; i < obj.size(); i++)
  // {
  //   std::string missing = url_decode_check_missing_fields(found, obj[i]);
  //   if (missing.size())
  //   {
  //     std::ostringstream ss;
  //     ss << '[' << i << "]" << missing;
  //     return ss.str();
  //   }
  // }
  return std::string();
}

template <typename... O>
std::string url_decode_check_missing_fields(const std::set<void*>& found, metamap<O...>& obj,
                                            bool root = false) {
  std::string missing;
  map(obj, [&](auto k, auto& v) {
    if (missing.size())
      return;
    missing = url_decode_check_missing_fields(found, v);
    if (missing.size())
      missing = (root ? "" : ".") + std::string(li::symbol_string(k)) + missing;
  });
  return missing;
}

template <typename S, typename... O>
std::string url_decode_check_missing_fields_on_subset(const std::set<void*>& found,
                                                      metamap<O...>& obj, bool root = false) {
  std::string missing;
  map(S(), [&](auto k, auto) {
    if (missing.size())
      return;
    auto& v = obj[k];
    missing = url_decode_check_missing_fields(found, v);
    if (missing.size())
      missing = (root ? "" : ".") + std::string(li::symbol_string(k)) + missing;
  });
  return missing;
}

// Frontend.
template <typename O> void url_decode(std::string_view str, O& obj) {
  std::set<void*> found;

  // Parse the urlencoded string
  while (str.size() > 0)
    str = url_decode2(found, str, obj, true);

  // Check for missing fields.
  std::string missing = url_decode_check_missing_fields(found, obj, true);
  if (missing.size())
    throw std::runtime_error(format_error("Missing argument ", missing));
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_URL_DECODE




namespace li {

struct http_request {

  http_request(http_async_impl::http_ctx& http_ctx) : http_ctx(http_ctx), fiber(http_ctx.fiber) {}
  
  inline std::string_view header(const char* k) const;
  inline std::string_view cookie(const char* k) const;

  // With list of parameters: s::id = int(), s::name = string(), ...
  template <typename S, typename V, typename... T>
  auto url_parameters(assign_exp<S, V> e, T... tail) const;
  template <typename S, typename V, typename... T>
  auto get_parameters(assign_exp<S, V> e, T... tail) const;
  template <typename S, typename V, typename... T>
  auto post_parameters(assign_exp<S, V> e, T... tail) const;

  // Const wrapper.
  template <typename O> auto url_parameters(const O& res) const;
  template <typename O> auto get_parameters(const O& res) const;
  template <typename O> auto post_parameters(const O& res) const;

  // With a metamap.
  template <typename O> auto url_parameters(O& res) const;
  template <typename O> auto get_parameters(O& res) const;
  template <typename O> auto post_parameters(O& res) const;

  http_async_impl::http_ctx& http_ctx;
  async_fiber_context& fiber;
  std::string url_spec;
};

struct url_parser_info_node {
  int slash_pos;
  bool is_path;
};
using url_parser_info = std::unordered_map<std::string, url_parser_info_node>;

auto make_url_parser_info(const std::string_view url) {

  url_parser_info info;

  auto check_pattern = [](const char* s, char a) { return *s == a and *(s + 1) == a; };

  int slash_pos = -1;
  for (int i = 0; i < int(url.size()); i++) {
    if (url[i] == '/')
      slash_pos++;
    // param must start with {{
    if (check_pattern(url.data() + i, '{')) {
      const char* param_name_start = url.data() + i + 2;
      const char* param_name_end = param_name_start;
      // param must end with }}
      while (!check_pattern(param_name_end, '}'))
        param_name_end++;

      if (param_name_end != param_name_start and check_pattern(param_name_end, '}')) {
        int size = param_name_end - param_name_start;
        bool is_path = false;
        if (size > 3 and param_name_end[-1] == '.' and param_name_end[-2] == '.' and
            param_name_end[-3] == '.') {
          is_path = true;
          param_name_end -= 3;
        }
        std::string_view param_name(param_name_start, param_name_end - param_name_start);
        info.emplace(param_name, url_parser_info_node{slash_pos, is_path});
      }
    }
  }
  return info;
}

template <typename O>
auto parse_url_parameters(const url_parser_info& fmt, const std::string_view url, O& obj) {
  // get the indexes of the slashes in url.
  std::vector<int> slashes;
  for (int i = 0; i < int(url.size()); i++) {
    if (url[i] == '/')
      slashes.push_back(i);
  }

  // For each field in O...
  //  find the location of the field in the url thanks to fmt.
  //  get it.
  map(obj, [&](auto k, auto v) {
    const char* symbol_str = symbol_string(k);
    auto it = fmt.find(symbol_str);
    if (it == fmt.end()) {
      throw std::runtime_error(std::string("Parameter ") + symbol_str + " not found in url " +
                               url.data());
    } else {
      // Location of the parameter in the url.
      int param_slash = it->second.slash_pos; // index of slash before param.
      if (param_slash >= int(slashes.size()))
        throw http_error::bad_request("Missing url parameter ", symbol_str);

      int param_start = slashes[param_slash] + 1;
      if (it->second.is_path) {
        if constexpr (std::is_same<std::decay_t<decltype(obj[k])>, std::string>::value or
                      std::is_same<std::decay_t<decltype(obj[k])>, std::string_view>::value) {
          obj[k] = std::string_view(url.data() + param_start - 1,
                                    url.size() - param_start + 1); // -1 to include the first /.
        } else {
          throw std::runtime_error(
              "{{path...}} parameters only accept std::string or std::string_view types.");
        }

      } else {
        int param_end = param_start;
        while (int(url.size()) > (param_end) and url[param_end] != '/')
          param_end++;

        std::string_view content(url.data() + param_start, param_end - param_start);
        try {
          if constexpr(std::is_same<std::remove_reference_t<decltype(v)>, std::string>::value or
                      std::is_same<std::remove_reference_t<decltype(v)>, std::string_view>::value)
            obj[k] = content;
          else
            obj[k] = boost::lexical_cast<decltype(v)>(content);
        } catch (const std::bad_cast& e) {
          throw http_error::bad_request("Cannot decode url parameter ", li::symbol_string(k), " : ",
                                        e.what());
        }
      }
    }
  });
  return obj;
}

inline std::string_view http_request::header(const char* k) const {
  return http_ctx.header(k);
}

inline std::string_view http_request::cookie(const char* k) const {
  return http_ctx.cookie(k);
  //FIXME return MHD_lookup_connection_value(mhd_connection, MHD_COOKIE_KIND, k);
}

template <typename S, typename V, typename... T>
auto http_request::url_parameters(assign_exp<S, V> e, T... tail) const {
  return url_parameters(mmm(e, tail...));
}

template <typename S, typename V, typename... T>
auto http_request::get_parameters(assign_exp<S, V> e, T... tail) const {
  return get_parameters(mmm(e, tail...));
}

template <typename S, typename V, typename... T>
auto http_request::post_parameters(assign_exp<S, V> e, T... tail) const {
  auto o = mmm(e, tail...);
  return post_parameters(o);
}

template <typename O> auto http_request::url_parameters(const O& res) const {
  O r;
  return url_parameters(r);
}

template <typename O> auto http_request::get_parameters(const O& res) const {
  O r;
  return get_parameters(r);
}
template <typename O> auto http_request::post_parameters(const O& res) const {
  O r;
  return post_parameters(r);
}

template <typename O> auto http_request::url_parameters(O& res) const {
  auto info = make_url_parser_info(url_spec);
  return parse_url_parameters(info, http_ctx.url(), res);
}

template <typename O> auto http_request::get_parameters(O& res) const {

  try {
    url_decode(http_ctx.get_parameters_string(), res);
  }
   catch (const std::runtime_error& e)
  {
    throw http_error::bad_request("Error while decoding the GET parameter: ", e.what());
  }

  return res;
}

template <typename O> auto http_request::post_parameters(O& res) const {
  try {
    std::string_view encoding = this->header("Content-Type");
    if (!encoding.data())
      throw http_error::bad_request(
          std::string("Content-Type is required to decode the POST parameters"));

    std::string_view body = http_ctx.read_whole_body();
    if (encoding == std::string_view("application/x-www-form-urlencoded"))
      url_decode(url_unescape(body), res);
    else if (encoding == std::string_view("application/json"))
      json_decode(body, res);
  } catch (std::exception e) {
    throw http_error::bad_request("Error while decoding the POST parameters: ", e.what());
  }

  return res;
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_REQUEST

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_RESPONSE
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_RESPONSE


//#include <stdlib.h>

#if defined(_MSC_VER)
#endif

//#include <sys/stat.h>

namespace li {
using namespace li;

struct http_response {
  inline http_response(http_async_impl::http_ctx& ctx) : http_ctx(ctx) {}

  inline void set_header(std::string_view k, std::string_view v) { http_ctx.set_header(k, v); }
  inline void set_cookie(std::string_view k, std::string_view v) { http_ctx.set_cookie(k, v); }

  template <typename O>
  inline void write_json(O&& obj) {     
    http_ctx.set_header("Content-Type", "application/json");
    http_ctx.respond_json(std::forward<O>(obj));
  }
  template <typename A, typename B, typename... O>
  void write_json(assign_exp<A, B>&& w1, O&&... ws) {
    write_json(mmm(std::forward<assign_exp<A, B>>(w1), std::forward<O>(ws)...));
  }

  inline void write() { http_ctx.respond(body); }
   void set_status(int s) { http_ctx.set_status(s); }

  template <typename A1, typename... A> inline void write(A1&& a1, A&&... a) {
    body += boost::lexical_cast<std::string>(std::forward<A1>(a1));
    write(std::forward<A>(a)...);
  }
  
  template <typename A1, typename... A> inline void write(std::string_view a1) 
  {
    http_ctx.respond(a1); 
  }

  inline void write_file(const std::string path) {
    http_ctx.send_static_file(path.c_str());
  }

  http_async_impl::http_ctx& http_ctx;
  std::string body;
};

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_RESPONSE


namespace li {

template <typename... O> auto http_serve(api<http_request, http_response> api, int port, O... opts) {

  auto options = mmm(opts...);

  int nthreads = get_or(options, s::nthreads, 4);

  auto handler = [api] (http_async_impl::http_ctx& ctx) {
    http_request rq{ctx};
    http_response resp(ctx);
    try {
      api.call(ctx.method(), ctx.url(), rq, resp);
    } catch (const http_error& e) {
      ctx.set_status(e.status());
      ctx.respond(e.what());
    } catch (const std::runtime_error& e) {
      std::cerr << "INTERNAL SERVER ERROR: "<< e.what() << std::endl;
      ctx.set_status(500);
      ctx.respond("Internal server error.");
    }
    ctx.respond_if_needed();
  };

  auto date_thread = std::make_shared<std::thread>([&] () {
      char a1[100];
      char a2[100];
      memset(a1, 0, sizeof(a1));
      memset(a2, 0, sizeof(a2));
      char* date_buf_tmp1 = a1;
      char* date_buf_tmp2 = a2;
      while (!quit_signal_catched)
      {
        time_t t = time(NULL);
        const tm& tm = *gmtime(&t);
        int size = strftime(date_buf_tmp1, sizeof(a1), "%a, %d %b %Y %T GMT", &tm);
        http_async_impl::date_buf = date_buf_tmp1;
        http_async_impl::date_buf_size = size;
        std::swap(date_buf_tmp1, date_buf_tmp2);
        usleep(1e6);
      }
    });

  auto server_thread = std::make_shared<std::thread>([=] () {
    std::cout << "Starting lithium::http_backend on port " << port << std::endl;
    //moustique_listen(port, SOCK_STREAM, nthreads, http_async_impl::make_http_processor(std::move(handler)));
    start_tcp_server(port, SOCK_STREAM, nthreads, http_async_impl::make_http_processor(std::move(handler)));
    date_thread->join();
  });

  if constexpr (has_key<decltype(options), s::non_blocking_t>())
  {
    usleep(0.1e6);
    date_thread->detach();
    server_thread->detach();
    //return mmm(s::server_thread = server_thread, s::date_thread = date_thread);
  }
  else
    server_thread->join();

}
}
#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_HTTP_LISTEN2


namespace li {
using http_api = api<http_request, http_response>;
}

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_HASHMAP_HTTP_SESSION
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_HASHMAP_HTTP_SESSION

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_RANDOM_COOKIE
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_RANDOM_COOKIE


namespace li {

std::string generate_secret_tracking_id() {
  std::ostringstream os;
  std::random_device rd;
  os << std::hex << rd() << rd() << rd() << rd();
  return os.str();
}

inline std::string random_cookie(http_request& request, http_response& response,
                                 const char* key = "silicon_token") {
  std::string token;
  std::string_view token_ = request.cookie(key);
  if (!token_.data()) {
    token = generate_secret_tracking_id();
    response.set_cookie(key, token);
  } else {
    std::cout << "got token " << token_ << std::endl;
    token = token_;
  }

  return token;
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_RANDOM_COOKIE


namespace li {

template <typename O> struct connected_hashmap_http_session {

  connected_hashmap_http_session(std::string session_id_, O* values_,
                                 std::unordered_map<std::string, O>& sessions_, std::mutex& mutex_)
      : session_id_(session_id_), values_(values_), sessions_(sessions_), mutex_(mutex_) {}

  // Store fiels into the session
  template <typename... F> auto store(F... fields) {
    map(mmm(fields...), [this](auto k, auto v) { values()[k] = v; });
  }

  // Access values of the session.
  O& values() {
    if (!values_)
      throw http_error::internal_server_error("Cannot access session values after logout.");
    return *values_;
  }

  // Delete the session from the database.
  void logout() {
    std::lock_guard<std::mutex> guard(mutex_);
    sessions_.erase(session_id_);
  }

private:
  std::string session_id_;
  O* values_;
  std::unordered_map<std::string, O>& sessions_;
  std::mutex& mutex_;
};

template <typename... F> struct hashmap_http_session {

  typedef decltype(mmm(std::declval<F>()...)) session_values_type;
  inline hashmap_http_session(std::string cookie_name, F... fields)
      : cookie_name_(cookie_name), default_values_(mmm(s::session_id = std::string(), fields...)) {}

  inline auto connect(http_request& request, http_response& response) {
    std::string session_id = random_cookie(request, response, cookie_name_.c_str());
    auto it = sessions_.try_emplace(session_id, default_values_).first;
    return connected_hashmap_http_session<session_values_type>{session_id, &it->second, sessions_,
                                                               sessions_mutex_};
  }

  std::string cookie_name_;
  session_values_type default_values_;
  std::unordered_map<std::string, session_values_type> sessions_;
  std::mutex sessions_mutex_;
};

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_HASHMAP_HTTP_SESSION

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_HTTP_AUTHENTICATION
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_HTTP_AUTHENTICATION


#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_SQL_HTTP_SESSION
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_SQL_HTTP_SESSION


namespace li {

template <typename ORM> struct connected_sql_http_session {

  // Construct the session.
  // Retrive the cookie
  // Retrieve it from the database.
  // Insert it if it does not exists.
  connected_sql_http_session(typename ORM::object_type& defaults, ORM&& orm,
                             const std::string& session_id)
      : loaded_(false), session_id_(session_id), orm_(std::forward<ORM>(orm)), values_(defaults) {}

  // Store fiels into the session
  template <typename... F> auto store(F... fields) {
    map(mmm(fields...), [this](auto k, auto v) { values_[k] = v; });
    if (!orm_.exists(s::session_id = session_id_))
      orm_.insert(s::session_id = session_id_, fields...);
    else
      orm_.update(s::session_id = session_id_, fields...);
  }

  // Access values of the session.
  const auto values() {
    load();
    return values_;
  }

  // Delete the session from the database.
  void logout() { orm_.remove(s::session_id = session_id_); }

private:
  auto load() {
    if (loaded_)
      return;
    if (auto new_values_ = orm_.find_one(s::session_id = session_id_))
      values_ = *new_values_;
    loaded_ = true;
  }

  bool loaded_;
  std::string session_id_;
  ORM orm_;
  typename ORM::object_type values_;
};

template <typename DB, typename... F>
decltype(auto) create_session_orm(DB& db, std::string table_name, F... fields) {
  return sql_orm_schema<DB>(db, table_name)
      .fields(s::session_id(s::read_only, s::primary_key) = sql_varchar<32>(), fields...);
}

template <typename DB, typename... F> struct sql_http_session {

  sql_http_session(DB& db, std::string table_name, std::string cookie_name, F... fields)
      : cookie_name_(cookie_name),
        default_values_(mmm(s::session_id = sql_varchar<32>(), fields...)),
        session_table_(create_session_orm(db, table_name, fields...)) {}

  auto connect(http_request& request, http_response& response) {
    return connected_sql_http_session(default_values_, session_table_.connect(request.fiber),
                                      random_cookie(request, response, cookie_name_.c_str()));
  }

  auto orm() { return session_table_; }

  std::string cookie_name_;
  std::decay_t<decltype(mmm(s::session_id = sql_varchar<32>(), std::declval<F>()...))>
      default_values_;
  std::decay_t<decltype(
      create_session_orm(std::declval<DB&>(), std::string(), std::declval<F>()...))>
      session_table_;
};

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_SQL_HTTP_SESSION


namespace li {

template <typename S, typename U, typename L, typename P, typename... CB>
struct http_authentication {
  http_authentication(S& session, U& users, L login_field, P password_field, CB... callbacks)
      : sessions_(session), users_(users), login_field_(login_field),
        password_field_(password_field), callbacks_(mmm(callbacks...)) {

    auto allowed_callbacks = mmm(s::hash_password, s::create_secret_key);

    static_assert(metamap_size<decltype(substract(callbacks_, allowed_callbacks))>() == 0,
                  "The only supported callbacks for http_authentication are: s::hash_password, "
                  "s::create_secret_key");
  }

  template <typename SS, typename... A> void call_callback(SS s, A&&... args) {
    if constexpr (has_key<decltype(callbacks_)>(s))
      return callbacks_[s](std::forward<A>(args)...);
  }

  bool login(http_request& req, http_response& resp) {
    auto lp = req.post_parameters(login_field_ = users_.all_fields()[login_field_],
                                  password_field_ = users_.all_fields()[password_field_]);

    if constexpr (has_key<decltype(callbacks_)>(s::hash_password))
      lp[password_field_] = callbacks_[s::hash_password](lp[login_field_], lp[password_field_]);

    if (auto user = users_.connect(req.fiber).find_one(lp)) {
      sessions_.connect(req, resp).store(s::user_id = user->id);
      return true;
    } else
      return false;
  }

  auto current_user(http_request& req, http_response& resp) {
    auto sess = sessions_.connect(req, resp);
    if (sess.values().user_id != -1)
      return users_.connect().find_one(s::id = sess.values().user_id);
    else
      return decltype(users_.connect(req.fiber).find_one(s::id = sess.values().user_id)){};
  }

  void logout(http_request& req, http_response& resp) { sessions_.connect(req, resp).logout(); }

  bool signup(http_request& req, http_response& resp) {
    auto new_user = req.post_parameters(users_.all_fields_except_computed());
    auto users = users_.connect(req.fiber);

    if (users.exists(login_field_ = new_user[login_field_]))
      return false;
    else {
      if constexpr (has_key<decltype(callbacks_)>(s::update_secret_key))
        callbacks_[s::update_secret_key](new_user[login_field_], new_user[password_field_]);
      if constexpr (has_key<decltype(callbacks_)>(s::hash_password))
        new_user[password_field_] =
            callbacks_[s::hash_password](new_user[login_field_], new_user[password_field_]);
      users.insert(new_user);
      return true;
    }
  }

  S& sessions_;
  U& users_;
  L login_field_;
  P password_field_;
  decltype(mmm(std::declval<CB>()...)) callbacks_;
};

template <typename... A> http_api http_authentication_api(http_authentication<A...>& auth) {

  http_api api;

  api.post("/login") = [&](http_request& request, http_response& response) {
    if (!auth.login(request, response))
      throw http_error::unauthorized("Bad login.");
  };

  api.get("/logout") = [&](http_request& request, http_response& response) {
    auth.logout(request, response);
  };

  api.get("/signup") = [&](http_request& request, http_response& response) {
    if (!auth.signup(request, response))
      throw http_error::bad_request("User already exists.");
  };

  return api;
}

// Disable this for now. (No time to install nettle on windows.)
// #include <nettle/sha3.h>
// inline std::string hash_sha3_512(const std::string& str)
// {
//   struct sha3_512_ctx ctx;
//   sha3_512_init(&ctx);
//   sha3_512_update(&ctx, str.size(), (const uint8_t*) str.data());
//   uint8_t h[SHA3_512_DIGEST_SIZE];
//   sha3_512_digest(&ctx, sizeof(h), h);
//   return std::string((const char*)h, sizeof(h));
// }

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_HTTP_AUTHENTICATION

//#include <li/http_backend/mhd.hh>
#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_SERVE_DIRECTORY
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_SERVE_DIRECTORY


namespace li {

auto serve_file(const std::string& root, std::string_view path, http_response& response) {
  std::string base_path = root;
  if (!base_path.empty() && base_path[base_path.size() - 1] != '/') {
    base_path.push_back('/');
  }

  static char dot = '.', slash = '/';

  // remove first slahs if needed.
  // std::string path(r->url.substr(prefix.string().size(), r->url.size() -
  // prefix.string().size()));
  size_t len = path.size();
  if (!path.empty() && path[0] == slash) {
    path = std::string_view(path.data() + 1, path.size() - 1);//erase(0, 1);
  }

  if (path.empty()) {
    throw http_error::bad_request("No file path given, directory listing not supported.");
  }

  if (len == 1 && path[0] == dot) {
    throw http_error::bad_request("Invalid URI ", path);
  } else if (len == 2 && path[0] == dot && path[1] == dot) {
    throw http_error::bad_request("Invalid URI ", path);
  } else {
    char prev0 = slash, prev1 = slash;
    for (size_t i = 0; i < len; ++i) {
      if (prev0 == dot && prev1 == dot && path[i] == slash) {
        throw http_error::bad_request("Unsupported URI, ../ is not allowed in the URI");
      }
      prev0 = prev1;
      prev1 = path[i];
    }
  }

  response.write_file(base_path + std::string(path));
};

inline auto serve_directory(std::string root) {
  http_api api;

  api.get("/{{path...}}") = [root](http_request& request, http_response& response) {
    auto path = request.url_parameters(s::path = std::string_view()).path;
    return serve_file(root, path, response);
  };
  return api;
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_SERVE_DIRECTORY

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_SQL_CRUD_API
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_SQL_CRUD_API


namespace li {

template <typename A, typename B, typename C>
auto sql_crud_api(sql_orm_schema<A, B, C>& orm_schema) {

  http_api api;

  api.post("/find_by_id") = [&](http_request& request, http_response& response) {
    auto params = request.post_parameters(s::id = int());
    if (auto obj = orm_schema.connect(request.fiber).find_one(s::id = params.id, request, response))
      response.write_json(obj);
    else
      throw http_error::not_found(orm_schema.table_name(), " with id ", params.id,
                                  " does not exist.");
  };

  api.post("/create") = [&](http_request& request, http_response& response) {
    auto insert_fields = orm_schema.all_fields_except_computed();
    auto obj = request.post_parameters(insert_fields);
    long long int id = orm_schema.connect(request.fiber).insert(obj, request, response);
    response.write_json(s::id = id);
  };

  api.post("/update") = [&](http_request& request, http_response& response) {
    auto obj = request.post_parameters(orm_schema.all_fields());
    orm_schema.connect(request.fiber).update(obj, request, response);
  };

  api.post("/remove") = [&](http_request& request, http_response& response) {
    auto obj = request.post_parameters(orm_schema.primary_key());
    orm_schema.connect(request.fiber).remove(obj, request, response);
  };

  return api;
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_SQL_CRUD_API


#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_HTTP_BENCHMARK
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_HTTP_BENCHMARK


namespace ctx = boost::context;

namespace li {

namespace http_benchmark_impl {

class timer {
public:
  void start() { start_ = std::chrono::high_resolution_clock::now(); }
  void end() { end_ = std::chrono::high_resolution_clock::now(); }

  unsigned long us() const {
    return std::chrono::duration_cast<std::chrono::microseconds>(end_ - start_).count();
  }

  unsigned long ms() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(end_ - start_).count();
  }

  unsigned long ns() const {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end_ - start_).count();
  }

private:
  std::chrono::time_point<std::chrono::high_resolution_clock> start_, end_;
};

void error(std::string msg) {
  perror(msg.c_str());
  exit(0);
}

} // namespace http_benchmark_impl

float http_benchmark(int NCONNECTIONS, int NTHREADS, int duration_in_ms, int port,
                     std::string_view req) {
  auto client_fn = [&](auto conn_handler) {
    int sockets[NCONNECTIONS];
    int portno, n;
    struct sockaddr_in serveraddr;
    struct hostent* server;
    const char* hostname;

    /* check command line arguments */
    hostname = "0.0.0.0";
    portno = port;

    /* socket: create the socket */
    for (int i = 0; i < NCONNECTIONS; i++) {
      sockets[i] = socket(AF_INET, SOCK_STREAM, 0);
      if (sockets[i] < 0)
        http_benchmark_impl::error("ERROR opening socket");
    }

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
      fprintf(stderr, "ERROR, no such host as %s\n", hostname);
      exit(0);
    }

    /* build the server's Internet address */
    bzero((char*)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char*)server->h_addr, (char*)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);



    int epoll_fd = epoll_create1(0);

    auto epoll_ctl = [epoll_fd](int fd, int op, uint32_t flags) {
      epoll_event event;
      event.data.fd = fd;
      event.events = flags;
      ::epoll_ctl(epoll_fd, op, fd, &event);
      return true;
    };

    // Set sockets non blocking.
    for (int i = 0; i < NCONNECTIONS; i++)
      fcntl(sockets[i], F_SETFL, fcntl(sockets[i], F_GETFL, 0) | O_NONBLOCK);

    for (int i = 0; i < NCONNECTIONS; i++)
      epoll_ctl(sockets[i], EPOLL_CTL_ADD, EPOLLIN | EPOLLOUT | EPOLLET);

    /* connect: create a connection with the server */
    std::vector<bool> opened(NCONNECTIONS, false);
    while (true)
    {
      for (int i = 0; i < NCONNECTIONS; i++)
      {
        if (opened[i]) continue;
        int ret = connect(sockets[i], (const sockaddr*)&serveraddr, sizeof(serveraddr));
        if (ret == 0)
          opened[i] = true;
        else if (errno == EINPROGRESS || errno == EALREADY)
          continue;
        else
          http_benchmark_impl::error(std::string("Cannot connect to server: ") + strerror(errno));
      }

      int nopened = 0;
      for (int i = 0; i < NCONNECTIONS; i++)
        nopened += opened[i];
      if (nopened == NCONNECTIONS)
        break;
    }

    const int MAXEVENTS = 64;
    std::vector<ctx::continuation> fibers;
    for (int i = 0; i < NCONNECTIONS; i++) {
      int infd = sockets[i];
      if (int(fibers.size()) < infd + 1)
        fibers.resize(infd + 10);

      fibers[infd] = ctx::callcc([fd = infd, &conn_handler, epoll_ctl](ctx::continuation&& sink) {
        auto read = [fd, &sink, epoll_ctl](char* buf, int max_size) {
          ssize_t count = ::recv(fd, buf, max_size, 0);
          while (count <= 0) {
            if ((count < 0 and errno != EAGAIN) or count == 0)
              return ssize_t(0);
            sink = sink.resume();
            count = ::recv(fd, buf, max_size, 0);
          }
          return count;
        };

        auto write = [fd, &sink, epoll_ctl](const char* buf, int size) {
          const char* end = buf + size;
          ssize_t count = ::send(fd, buf, end - buf, MSG_NOSIGNAL);
          if (count > 0)
            buf += count;
          while (buf != end) {
            if ((count < 0 and errno != EAGAIN) or count == 0)
              return false;
            sink = sink.resume();
            count = ::send(fd, buf, end - buf, MSG_NOSIGNAL);
            if (count > 0)
              buf += count;
          }
          return true;
        };
        conn_handler(fd, read, write);
        return std::move(sink);
      });
    }

    // Even loop.
    epoll_event events[MAXEVENTS];
    http_benchmark_impl::timer global_timer;
    global_timer.start();
    global_timer.end();
    while (global_timer.ms() < duration_in_ms) {
      int n_events = epoll_wait(epoll_fd, events, MAXEVENTS, 1);
      for (int i = 0; i < n_events; i++) {
        if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
        } else // Data available on existing sockets. Wake up the fiber associated with
               // events[i].data.fd.
          fibers[events[i].data.fd] = fibers[events[i].data.fd].resume();
      }

      global_timer.end();
    }

    // Close all client sockets.
    for (int i = 0; i < NCONNECTIONS; i++)
      close(sockets[i]);
  };

  std::atomic<int> nmessages = 0;

  auto bench_tcp = [&]() {
    client_fn([&](int fd, auto read, auto write) { // flood the server.
      while (true) {
        char buf_read[10000];
        write(req.data(), req.size());
        int rd = read(buf_read, sizeof(buf_read));
        nmessages++;
      }
    });
  };

  int nthreads = NTHREADS;
  std::vector<std::thread> ths;
  for (int i = 0; i < nthreads; i++)
    ths.push_back(std::thread(bench_tcp));
  for (auto& t : ths)
    t.join();
  return (1000. * nmessages / duration_in_ms);
}

} // namespace li
#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_HTTP_BENCHMARK


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_HTTP_BACKEND

