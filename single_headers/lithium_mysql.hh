// Author: Matthieu Garrigues matthieu.garrigues@gmail.com
//
// Single header version the lithium_mysql library.
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
#include <lithium_symbol.hh>
#include <map>
#include <memory>
#include <mutex>
#include <mysql.h>
#include <optional>
#include <sstream>
#include <string>
#include <sys/epoll.h>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>


#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_DATABASE_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_DATABASE_HH


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

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_CAT_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_CAT_HH

namespace li {

template <typename... T, typename... U>
inline decltype(auto) cat(const metamap<T...>& a, const metamap<U...>& b) {
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

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_MAKE_HH


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

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_MAKE_METAMAP_SKIP_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_MAP_REDUCE_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_MAP_REDUCE_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_TUPLE_UTILS_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_TUPLE_UTILS_HH


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
#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_TUPLE_UTILS_HH


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

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_MAP_REDUCE_HH



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

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_INTERSECTION_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_SUBSTRACT_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_SUBSTRACT_HH


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

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_SUBSTRACT_HH


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_METAMAP_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_ASYNC_WRAPPER_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_ASYNC_WRAPPER_HH


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

#ifndef LI_SYMBOL_write_access
#define LI_SYMBOL_write_access
    LI_SYMBOL(write_access)
#endif


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SYMBOLS_HH


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

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_ASYNC_WRAPPER_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_STATEMENT_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_STATEMENT_HH



#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_INTERNAL_MYSQL_BIND_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_INTERNAL_MYSQL_BIND_HH



#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_INTERNAL_MYSQL_STATEMENT_DATA_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_INTERNAL_MYSQL_STATEMENT_DATA_HH


/**
 * @brief Store the data that mysql_statement holds.
 *
 */
struct mysql_statement_data : std::enable_shared_from_this<mysql_statement_data> {

  MYSQL_STMT* stmt_ = nullptr;
  int num_fields_ = -1;
  MYSQL_RES* metadata_ = nullptr;
  MYSQL_FIELD* fields_ = nullptr;

  mysql_statement_data(MYSQL_STMT* stmt) {
    stmt_ = stmt;
    metadata_ = mysql_stmt_result_metadata(stmt_);
    if (metadata_) {
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

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_INTERNAL_MYSQL_STATEMENT_DATA_HH


namespace li {

// Convert C++ types to mysql types.
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

// Convert c++ type to mysql types.
template <typename T>
typename std::enable_if_t<std::is_integral<T>::value, std::string> cpptype_to_mysql_type(const T&) {
  return "INT";
}
template <typename T>
typename std::enable_if_t<std::is_floating_point<T>::value, std::string> cpptype_to_mysql_type(const T&) {
  return "DOUBLE";
}
inline std::string cpptype_to_mysql_type(const std::string&) { return "MEDIUMTEXT"; }
inline std::string cpptype_to_mysql_type(const sql_blob&) { return "BLOB"; }
template <unsigned S> inline std::string cpptype_to_mysql_type(const sql_varchar<S>) {
  std::ostringstream ss;
  ss << "VARCHAR(" << S << ')';
  return ss.str();
}

// Bind parameter functions
// Used to bind input parameters of prepared statement.
template <unsigned N> struct mysql_bind_data {
  mysql_bind_data() {
     memset(bind.data(), 0, N * sizeof(MYSQL_BIND));
     for (int i = 0; i < N; i++) bind[i].error = &errors[i];
  }
  std::array<unsigned long, N> real_lengths;
  std::array<MYSQL_BIND, N> bind;
  std::array<char, N> errors;
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
void mysql_bind_param(MYSQL_BIND& b, const std::string& s) {
  mysql_bind_param(b, *const_cast<std::string*>(&s));
}

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
void mysql_bind_param(MYSQL_BIND& b, const sql_blob& s) {
  mysql_bind_param(b, *const_cast<sql_blob*>(&s));
}

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
mysql_bind_data<sizeof...(A)> mysql_bind_output(mysql_statement_data& data, metamap<A...>& o) {
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
mysql_bind_data<sizeof...(A)> mysql_bind_output(mysql_statement_data& data, std::tuple<A...>& o) {
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

// Forward reference tuple impl.
template <typename... A>
mysql_bind_data<sizeof...(A)> mysql_bind_output(mysql_statement_data& data, std::tuple<A...>&& o) {
  if (data.num_fields_ != sizeof...(A))
    throw std::runtime_error("mysql_statement error: The number of column in the result set does "
                             "not match the number of attributes of the tuple to bind.");

  mysql_bind_data<sizeof...(A)> bind_data;
  MYSQL_BIND* bind = bind_data.bind.data();
  unsigned long* real_lengths = bind_data.real_lengths.data();

  int i = 0;
  tuple_map(std::forward<std::tuple<A...>>(o), [&](auto& m) {
    mysql_bind_output(bind[i], real_lengths + i, m);
    i++;
  });

  return bind_data;
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_INTERNAL_MYSQL_BIND_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_RESULT_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_RESULT_HH


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

template <typename B> template <typename F> void sql_result<B>::map(F map_function) {

  typedef typename unconstref_tuple_elements<callable_arguments_tuple_t<F>>::ret TP;

  auto t = [] {
    static_assert(std::tuple_size_v<TP> > 0, "sql_result map function must take at least 1 argument.");

    if constexpr (std::tuple_size_v<TP> == 1)
      return std::tuple_element_t<0, TP>{};
    else if constexpr (std::tuple_size_v<TP> > 1)
      return TP{};
  }();


  while (auto res = this->read_optional<decltype(t)>()) {
    if constexpr (std::tuple_size<TP>::value == 1)
      map_function(res.value());
    else if constexpr (std::tuple_size<TP>::value > 1)
      std::apply(map_function, res.value());
  }
}
} // namespace li
#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_RESULT_HPP


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_RESULT_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_STATEMENT_RESULT_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_STATEMENT_RESULT_HH




namespace li {

/**
 * @brief The prepared statement result interface.
 *
 * @tparam B
 */
template <typename B> struct mysql_statement_result {

  mysql_statement_result& operator=(mysql_statement_result&) = delete;
  mysql_statement_result(const mysql_statement_result&) = delete;

  /**
   * @brief Destructor. Free the result if needed.
   */
  inline ~mysql_statement_result() { flush_results(); }

  inline void flush_results() {
    if (result_allocated_)
      mysql_wrapper_.mysql_stmt_free_result(connection_status_, data_.stmt_);
    result_allocated_ = false;
  }

  // Read std::tuple and li::metamap.
  template <typename T> bool read(T&& output);

  /**
   * @return the number of rows affected by the request.
   */
  long long int affected_rows();

  /**
   * @brief Return the last id generated by a insert comment.
   *
   * @return the last inserted id.
   */
  long long int last_insert_id();

  void next_row();

  // Internal methods.
  template <typename... A>
  void finalize_fetch(MYSQL_BIND* bind, unsigned long* real_lengths, metamap<A...>& o);
  template <typename... A>
  void finalize_fetch(MYSQL_BIND* bind, unsigned long* real_lengths, std::tuple<A...>& o);
  template <typename T> void fetch_column(MYSQL_BIND*, unsigned long, T&, int);

  void fetch_column(MYSQL_BIND* b, unsigned long real_length, std::string& v, int i);
  template <unsigned SIZE>
  void fetch_column(MYSQL_BIND& b, unsigned long real_length, sql_varchar<SIZE>& v, int i);
  template <typename T> int fetch(T&& o);

  B& mysql_wrapper_;
  mysql_statement_data& data_;
  std::shared_ptr<int> connection_status_;
  bool result_allocated_ = false;
};

} // namespace li

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_STATEMENT_RESULT_HPP
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_STATEMENT_RESULT_HPP


namespace li {

template <typename B> long long int mysql_statement_result<B>::affected_rows() {
  return mysql_stmt_affected_rows(data_.stmt_);
}

template <typename B>
template <typename T>
void mysql_statement_result<B>::fetch_column(MYSQL_BIND* b, unsigned long, T&, int) {
  if (*b->error) {
    throw std::runtime_error("Result could not fit in the provided types: loss of sign or significant digits or type mismatch.");
  }
}

template <typename B>
void mysql_statement_result<B>::fetch_column(MYSQL_BIND* b, unsigned long real_length,
                                             std::string& v, int i) {
  // If the string was big enough to hold the result string, return it.
  if (real_length <= v.size()) {
    v.resize(real_length);
    return;
  }
  // Otherwise we need to call mysql_stmt_fetch_column again to get the result string.

  // Reserve enough space to fetch the string.
  v.resize(real_length);

  // Bind result.
  b[i].buffer_length = real_length;
  b[i].buffer = &v[0];
  mysql_stmt_bind_result(data_.stmt_, b);
  result_allocated_ = true;

  if (mysql_stmt_fetch_column(data_.stmt_, b + i, i, 0) != 0) {
    *connection_status_ = 1;
    throw std::runtime_error(std::string("mysql_stmt_fetch_column error: ") +
                             mysql_stmt_error(data_.stmt_));
  }
}

template <typename B>
template <unsigned SIZE>
void mysql_statement_result<B>::fetch_column(MYSQL_BIND& b, unsigned long real_length,
                                             sql_varchar<SIZE>& v, int i) {
  v.resize(real_length);
}

template <typename B> template <typename T> bool mysql_statement_result<B>::read(T&& output) {
  // Todo: skip useless mysql_bind_output and mysql_stmt_bind_result if too costly.
  // if (bound_ptr_ == (void*)output)...
  try {

    // Bind output.
    auto bind_data = mysql_bind_output(data_, std::forward<T>(output));
    unsigned long* real_lengths = bind_data.real_lengths.data();
    MYSQL_BIND* bind = bind_data.bind.data();

    bool bind_ret = mysql_stmt_bind_result(data_.stmt_, bind_data.bind.data());
    // std::cout << "bind_ret: " << bind_ret << std::endl;
    if (bind_ret != 0)
    {
      throw std::runtime_error(std::string("mysql_stmt_bind_result error: ") + mysql_stmt_error(data_.stmt_));
    }

    result_allocated_ = true;

    // Fetch row.
    // Note: This also advance to the next row.
    int res = mysql_wrapper_.mysql_stmt_fetch(connection_status_, data_.stmt_);
    if (res == MYSQL_NO_DATA) // If end of result, return false.
      return false;

    // Finalize fetch:
    //    - fetch strings that did not fit the preallocated strings.
    //    - check for truncated data errors.
    //    - resize preallocated strings that were bigger than the request result.
    if constexpr (is_tuple<T>::value) {
      int i = 0;
      tuple_map(std::forward<T>(output), [&](auto& m) {
        this->fetch_column(bind, real_lengths[i], m, i);
        i++;
      });
    } else {
      li::map(std::forward<T>(output), [&](auto k, auto& v) {
        for (int i = 0; i < data_.num_fields_; i++)
          if (!strcmp(data_.fields_[i].name, li::symbol_string(k)))
            this->fetch_column(bind, real_lengths[i], v, i);
      });
    }
    return true;
  } catch (const std::runtime_error& e) {
    mysql_stmt_reset(data_.stmt_);
    throw e;
  }

}

template <typename B> long long int mysql_statement_result<B>::last_insert_id() {
  return mysql_stmt_insert_id(data_.stmt_);
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_STATEMENT_RESULT_HPP


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_STATEMENT_RESULT_HH


namespace li {

/**
 * @brief Mysql prepared statement
 *
 * @tparam B the blocking or non blocking mode.
 */
template <typename B> struct mysql_statement {

  /**
   * @brief Execute the statement with argument.
   * Number of args must be equal to the number of placeholders in the request.
   *
   * @param args the arguments
   * @return mysql_statement_result<B> the result
   */
  template <typename... T> sql_result<mysql_statement_result<B>> operator()(T&&... args);

  B& mysql_wrapper_;
  mysql_statement_data& data_;
  std::shared_ptr<int> connection_status_;
};

} // namespace li

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_STATEMENT_HPP
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_STATEMENT_HPP


namespace li {

template <typename B>
template <typename... T>
sql_result<mysql_statement_result<B>> mysql_statement<B>::operator()(T&&... args) {

  if (sizeof...(T) > 0) {
    // Bind the ...args in the MYSQL BIND structure.
    MYSQL_BIND bind[sizeof...(T)];
    memset(bind, 0, sizeof(bind));
    int i = 0;
    tuple_map(std::forward_as_tuple(args...), [&](auto& m) {
      mysql_bind_param(bind[i], m);
      i++;
    });

    // Pass MYSQL BIND to mysql.
    if (mysql_stmt_bind_param(data_.stmt_, bind) != 0) {
      *connection_status_ = 1;
      throw std::runtime_error(std::string("mysql_stmt_bind_param error: ") +
                               mysql_stmt_error(data_.stmt_));
    }
  }
  
  // Execute the statement.
  mysql_wrapper_.mysql_stmt_execute(connection_status_, data_.stmt_);

  // Return the wrapped mysql result.
  return sql_result<mysql_statement_result<B>>{
      mysql_statement_result<B>{mysql_wrapper_, data_, connection_status_}};
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_STATEMENT_HPP


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_STATEMENT_HH


#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_CONNECTION_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_CONNECTION_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_RESULT_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_RESULT_HH



namespace li {

/**
 * @brief Store a access to the result of a sql query (non prepared).
 *
 * @tparam B must be mysql_functions_blocking or mysql_functions_non_blocking
 */
template <typename B> struct mysql_result {

  B& mysql_wrapper_; // blocking or non blockin mysql functions wrapper.
  MYSQL* con_; // Mysql connection
  std::shared_ptr<int> connection_status_; // Status of the connection
  MYSQL_RES* result_ = nullptr; // Mysql result.

  unsigned long* current_row_lengths_ = nullptr;
  MYSQL_ROW current_row_ = nullptr;
  bool end_of_result_ = false;
  int current_row_num_fields_ = 0;
  
  mysql_result& operator=(mysql_result&) = delete;
  mysql_result(const mysql_result&) = delete;

  inline ~mysql_result() { flush(); }

  inline void flush() { if (result_) { mysql_free_result(result_); result_ = nullptr; } } 
  inline void flush_results() { this->flush(); }
  inline void next_row();
  template <typename T> bool read(T&& output);

  /**
   * @return the number of rows affected by the request.
   */
  long long int affected_rows();

  /**
   * @brief Return the last id generated by a insert comment.
   *
   * @return the last inserted id.
   */
  long long int last_insert_id();

};

} // namespace li

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_RESULT_HPP
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_RESULT_HPP



namespace li {

template <typename B> void mysql_result<B>::next_row() {

  if (!result_)
    result_ = mysql_use_result(con_);
  current_row_ = mysql_wrapper_.mysql_fetch_row(connection_status_, result_);
  current_row_num_fields_ = mysql_num_fields(result_);
  if (!current_row_ || current_row_num_fields_ == 0) {
    end_of_result_ = true;
    return;
  }

  current_row_lengths_ = mysql_fetch_lengths(result_);
}

template <typename B> template <typename T> bool mysql_result<B>::read(T&& output) {

  next_row();

  if (end_of_result_)
    return false;

  if constexpr (is_tuple<T>::value) { // Tuple

    if (std::tuple_size_v<std::decay_t<T>> != current_row_num_fields_)
      throw std::runtime_error(std::string("The request number of field (") +
                               boost::lexical_cast<std::string>(current_row_num_fields_) +
                               ") does not match the size of the tuple (" +
                               boost::lexical_cast<std::string>(std::tuple_size_v<std::decay_t<T>>) + ")");
    int i = 0;
    li::tuple_map(std::forward<T>(output), [&](auto& v) {
      // std::cout << "read " << std::string_view(current_row_[i], current_row_lengths_[i]) << std::endl;
      v = boost::lexical_cast<std::decay_t<decltype(v)>>(
          std::string_view(current_row_[i], current_row_lengths_[i]));
      i++;
    });

  } else { // Metamap.

    if (li::metamap_size(output) != current_row_num_fields_)
      throw std::runtime_error(
          "The request number of field does not match the size of the metamap object.");
    int i = 0;
    li::map(std::forward<T>(output), [&](auto k, auto& v) {
      v = boost::lexical_cast<decltype(v)>(
          std::string_view(current_row_[i], current_row_lengths_[i]));
      i++;
    });
  }

  return true;
}

template <typename B> long long int mysql_result<B>::affected_rows() {
  return mysql_affected_rows(con_);
}

template <typename B> long long int mysql_result<B>::last_insert_id() {
  return mysql_insert_id(con_);
}

} // namespace li


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_RESULT_HPP


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_RESULT_HH


namespace li {

// Forward ref.
struct mysql_connection_data;

struct mysql_tag {};

template <typename B> // must be mysql_functions_blocking or mysql_functions_non_blocking
struct mysql_connection {

  typedef mysql_tag db_tag;

  /**
   * @brief Construct a new mysql connection object.
   *
   * @param mysql_wrapper the [non]blocking mysql wrapper.
   * @param data the connection data.
   */
  template <typename P>
  inline mysql_connection(B mysql_wrapper, std::shared_ptr<li::mysql_connection_data> data,
  P put_data_back_in_pool);

  /**
   * @brief Last inserted row id.
   *
   * @return long long int the row id.
   */
  long long int last_insert_rowid();

  /**
   * @brief Execute a SQL request.
   *
   * @param rq the request string
   * @return mysql_result<B> the result.
   */
  sql_result<mysql_result<B>> operator()(const std::string& rq);

  /**
   * @brief Build a sql prepared statement.
   *
   * @param rq the request string
   * @return mysql_statement<B> the statement.
   */
  mysql_statement<B> prepare(const std::string& rq);

  /**
   * @brief Build or retrieve a sql statement from the connection cache.
   * Will regenerate the statement if one of the @param keys changed.
   *
   * @param f the function that generate the statement.
   * @param keys the keys.
   * @return mysql_statement<B> the statement.
   */
  template <typename F, typename... K> mysql_statement<B> cached_statement(F f, K... keys);

  template <typename T> std::string type_to_string(const T& t) { return cpptype_to_mysql_type(t); }

  B mysql_wrapper_;
  std::shared_ptr<mysql_connection_data> data_;
  std::shared_ptr<int> connection_status_;
};

/**
 * @brief Data of a connection.
 *
 */
struct mysql_connection_data {

  ~mysql_connection_data();

  MYSQL* connection_;
  std::unordered_map<std::string, std::shared_ptr<mysql_statement_data>> statements_;
  type_hashmap<std::shared_ptr<mysql_statement_data>> statements_hashmap_;
};

} // namespace li

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_CONNECTION_HPP
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_CONNECTION_HPP



namespace li {

mysql_connection_data::~mysql_connection_data() { mysql_close(connection_); }

template <typename B>
template <typename P>
inline mysql_connection<B>::mysql_connection(B mysql_wrapper,
                                             std::shared_ptr<li::mysql_connection_data> data,
                                             P put_data_back_in_pool)
    : mysql_wrapper_(mysql_wrapper), data_(data) {

  connection_status_ =
      std::shared_ptr<int>(new int(0), [put_data_back_in_pool, data](int* p) mutable { put_data_back_in_pool(data, *p); });
}

template <typename B> long long int mysql_connection<B>::last_insert_rowid() {
  return mysql_insert_id(data_->connection_);
}

template <typename B>
sql_result<mysql_result<B>> mysql_connection<B>::operator()(const std::string& rq) {
  mysql_wrapper_.mysql_real_query(connection_status_, data_->connection_, rq.c_str(), rq.size());
  return sql_result<mysql_result<B>>{
      mysql_result<B>{mysql_wrapper_, data_->connection_, connection_status_}};
}

template <typename B>
template <typename F, typename... K>
mysql_statement<B> mysql_connection<B>::cached_statement(F f, K... keys) {
  if (data_->statements_hashmap_(f).get() == nullptr) {
    mysql_statement<B> res = prepare(f());
    data_->statements_hashmap_(f, keys...) = res.data_.shared_from_this();
    return res;
  } else
    return mysql_statement<B>{mysql_wrapper_, *data_->statements_hashmap_(f, keys...),
                              connection_status_};
}

template <typename B> mysql_statement<B> mysql_connection<B>::prepare(const std::string& rq) {
  auto it = data_->statements_.find(rq);
  if (it != data_->statements_.end()) {
    // mysql_wrapper_.mysql_stmt_free_result(it->second->stmt_);
    // mysql_wrapper_.mysql_stmt_reset(it->second->stmt_);
    return mysql_statement<B>{mysql_wrapper_, *it->second, connection_status_};
  }
  // std::cout << "prepare " << rq << std::endl;
  MYSQL_STMT* stmt = mysql_stmt_init(data_->connection_);
  if (!stmt) {
    *connection_status_ = true;
    throw std::runtime_error(std::string("mysql_stmt_init error: ") +
                             mysql_error(data_->connection_));
  }
  if (mysql_wrapper_.mysql_stmt_prepare(connection_status_, stmt, rq.data(), rq.size())) {
    *connection_status_ = true;
    throw std::runtime_error(std::string("mysql_stmt_prepare error: ") +
                             mysql_error(data_->connection_));
  }

  auto pair = data_->statements_.emplace(rq, std::make_shared<mysql_statement_data>(stmt));
  return mysql_statement<B>{mysql_wrapper_, *pair.first->second, connection_status_};
}

} // namespace li


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_CONNECTION_HPP


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_CONNECTION_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_DATABASE_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_DATABASE_HH


namespace li {
// thread local map of sql_database<I> -> sql_database_thread_local_data<I>;
// This is used to store the thread local async connection pool.
thread_local std::unordered_map<int, void*> sql_thread_local_data;
thread_local int database_id_counter = 0;

template <typename I> struct sql_database_thread_local_data {

  typedef typename I::connection_data_type connection_data_type;

  // Async connection pools.
  std::deque<std::shared_ptr<connection_data_type>> async_connections_;

  int n_connections_on_this_thread_ = 0;
};

struct active_yield {
  typedef std::runtime_error exception_type;
  void epoll_add(int, int) {}
  void epoll_mod(int, int) {}
  void yield() {}
};

template <typename I> struct sql_database {
  I impl;

  typedef typename I::connection_data_type connection_data_type;
  typedef typename I::db_tag db_tag;

  // Sync connections pool.
  std::deque<std::shared_ptr<connection_data_type>> sync_connections_;
  // Sync connections mutex.
  std::mutex sync_connections_mutex_;

  int n_sync_connections_ = 0;
  int max_sync_connections_ = 0;
  int max_async_connections_per_thread_ = 0;
  int database_id_ = 0;

  template <typename... O> sql_database(O&&... opts) : impl(std::forward<O>(opts)...) {
    auto options = mmm(opts...);
    max_async_connections_per_thread_ = get_or(options, s::max_async_connections_per_thread, 200);
    max_sync_connections_ = get_or(options, s::max_sync_connections, 2000);

    this->database_id_ = database_id_counter++;
  }

  ~sql_database() {
    auto it = sql_thread_local_data.find(this->database_id_);
    if (it != sql_thread_local_data.end())
    {
      delete (sql_database_thread_local_data<I>*) sql_thread_local_data[this->database_id_];
      sql_thread_local_data.erase(this->database_id_);
    }
  }

  auto& thread_local_data() {
    auto it = sql_thread_local_data.find(this->database_id_);
    if (it == sql_thread_local_data.end())
    {
      auto data = new sql_database_thread_local_data<I>;
      sql_thread_local_data[this->database_id_] = data;
      return *data;
    }
    else
      return *(sql_database_thread_local_data<I>*) it->second;
  }
  /**
   * @brief Build aa new database connection. The connection provides RAII: it will be
   * placed back in the available connection pool whenever its constructor is called.
   *
   * @param fiber the fiber object providing the 3 non blocking logic methods:
   *
   *    - void epoll_add(int fd, int flags); // Make the current epoll fiber wakeup on
   *                                            file descriptor fd
   *    - void epoll_mod(int fd, int flags); // Modify the epoll flags on file
   *                                            descriptor fd
   *    - void yield() // Yield the current epoll fiber.
   *
   * @return the new connection.
   */
  template <typename Y> inline auto connect(Y& fiber) {

    auto pool = [this] {

      if constexpr (std::is_same_v<Y, active_yield>) // Synchonous mode
        return make_metamap_reference(
            s::connections = this->sync_connections_,
            s::n_connections = this->n_sync_connections_,
            s::max_connections = this->max_sync_connections_);
      else  // Asynchonous mode
        return make_metamap_reference(
            s::connections = this->thread_local_data().async_connections_,
            s::n_connections =  this->thread_local_data().n_connections_on_this_thread_,
            s::max_connections = this->max_async_connections_per_thread_);
    }();

    std::shared_ptr<connection_data_type> data = nullptr;
    while (!data) {

      if (!pool.connections.empty()) {
        auto lock = [&pool, this] {
          if constexpr (std::is_same_v<Y, active_yield>)
            return std::lock_guard<std::mutex>(this->sync_connections_mutex_);
          else return 0;
        }();
        data = pool.connections.back();
        pool.connections.pop_back();
        fiber.epoll_add(impl.get_socket(data), EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET);
      } else {
        if (pool.n_connections >= pool.max_connections) {
          if constexpr (std::is_same_v<Y, active_yield>)
            throw std::runtime_error("Maximum number of sql connection exeeded.");
          else
            // std::cout << "Waiting for a free sql connection..." << std::endl;
            fiber.yield();
          continue;
        }
        pool.n_connections++;

        try {

          data = impl.new_connection(fiber);
        } catch (typename Y::exception_type& e) {
          pool.n_connections--;
          throw std::move(e);
        }

        if (!data)
          pool.n_connections--;
      }
    }
    assert(data);
    return impl.scoped_connection(
        fiber, data, [pool, this](std::shared_ptr<connection_data_type> data, int error) {
          if (!error && pool.connections.size() < pool.max_connections) {
            auto lock = [&pool, this] {
              if constexpr (std::is_same_v<Y, active_yield>)
                return std::lock_guard<std::mutex>(this->sync_connections_mutex_);
              else return 0;
            }();

            pool.connections.push_back(data);
            
          } else {
            if (pool.connections.size() > pool.max_connections)
              std::cerr << "Error: connection pool size " << pool.connections.size()
                        << " exceed pool max_connections " << pool.max_connections << std::endl;
            pool.n_connections--;
          }
        });
  }

  /**
   * @brief Provide a new mysql blocking connection. The connection provides RAII: it will be
   * placed back in the available connection pool whenver its constructor is called.
   *
   * @return the connection.
   */
  inline auto connect() { active_yield yield; return this->connect(yield); }
};

}
#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_DATABASE_HH


namespace li {

struct mysql_database_impl {

  typedef mysql_tag db_tag;
  typedef mysql_connection_data connection_data_type;

  /**
   * @brief Construct a new mysql database object
   *
   *
   * @tparam O
   * @param opts Available options are:
   *                 - s::host = "hostname or ip"
   *                 - s::database = "database name"
   *                 - s::user = "username"
   *                 - s::password = "user passord"
   *                 - s::charset = "character set" default: utf8
   *
   */
  template <typename... O> inline mysql_database_impl(O... opts);

  inline ~mysql_database_impl();

  template <typename Y> inline std::shared_ptr<mysql_connection_data> new_connection(Y& fiber);
  inline int get_socket(std::shared_ptr<mysql_connection_data> data);

  template <typename Y, typename F>
  inline auto scoped_connection(Y& fiber, std::shared_ptr<mysql_connection_data>& data,
                                F put_back_in_pool);

  std::string host_, user_, passwd_, database_;
  unsigned int port_;
  std::string character_set_;
};

typedef sql_database<mysql_database_impl> mysql_database;

} // namespace li

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_DATABASE_HPP
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_DATABASE_HPP



namespace li {

template <typename... O> inline mysql_database_impl::mysql_database_impl(O... opts) {

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
  port_ = get_or(options, s::port, 3306);
  character_set_ = get_or(options, s::charset, "utf8");

  if (mysql_library_init(0, NULL, NULL))
    throw std::runtime_error("Could not initialize MySQL library.");
  if (!mysql_thread_safe())
    throw std::runtime_error("Mysql is not compiled as thread safe.");
}

mysql_database_impl::~mysql_database_impl() { mysql_library_end(); }

inline int mysql_database_impl::get_socket(std::shared_ptr<mysql_connection_data> data) {
  return mysql_get_socket(data->connection_);
}

template <typename Y>
inline std::shared_ptr<mysql_connection_data> mysql_database_impl::new_connection(Y& fiber) {

  MYSQL* mysql;
  int mysql_fd = -1;
  int status;
  MYSQL* connection;

  mysql = new MYSQL;
  mysql_init(mysql);

  if constexpr (std::is_same_v<Y, active_yield>) { // Synchronous connection
    connection = mysql;
    connection = mysql_real_connect(connection, host_.c_str(), user_.c_str(), passwd_.c_str(),
                                    database_.c_str(), port_, NULL, 0);
    if (!connection)
      return nullptr;
  } else { // Async connection.
    mysql_options(mysql, MYSQL_OPT_NONBLOCK, 0);
    connection = nullptr;
    status = mysql_real_connect_start(&connection, mysql, host_.c_str(), user_.c_str(),
                                      passwd_.c_str(), database_.c_str(), port_, NULL, 0);

    // std::cout << "after: " << mysql_get_socket(mysql) << " " << status == MYSQL_ <<
    // std::endl;
    mysql_fd = mysql_get_socket(mysql);
    if (mysql_fd == -1) {
      // std::cout << "Invalid mysql connection bad mysql_get_socket " << status << " " << mysql
      // << std::endl;
      mysql_close(mysql);
      return nullptr;
    }

    if (status)
      fiber.epoll_add(mysql_fd, EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET);
    while (status)
      try {
        fiber.yield();
        status = mysql_real_connect_cont(&connection, mysql, status);
      } catch (typename Y::exception_type& e) {
        // Yield thrown a exception (probably because a closed connection).
        // std::cerr << "Warning: yield threw an exception while connecting to mysql: "
        //  << total_number_of_mysql_connections << std::endl;
        mysql_close(mysql);
        throw std::move(e);
      }
    if (!connection) {
      // Error in mysql_real_connect_cont
      return nullptr;
    }
  }

  mysql_set_character_set(mysql, character_set_.c_str());
  return std::shared_ptr<mysql_connection_data>(new mysql_connection_data{mysql});
}

template <typename Y, typename F>
inline auto mysql_database_impl::scoped_connection(Y& fiber,
                                                   std::shared_ptr<mysql_connection_data>& data,
                                                   F put_back_in_pool) {
  if constexpr (std::is_same_v<active_yield, Y>)
    return mysql_connection(mysql_functions_blocking{}, data, put_back_in_pool);

  else
    return mysql_connection(mysql_functions_non_blocking<Y>{fiber}, data, put_back_in_pool);
}

} // namespace li
#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_DATABASE_HPP


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_DATABASE_HH


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_MYSQL_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_ORM_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_ORM_HH

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_SYMBOLS_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_SYMBOLS_HH

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

#ifndef LI_SYMBOL_ssl_certificate
#define LI_SYMBOL_ssl_certificate
    LI_SYMBOL(ssl_certificate)
#endif

#ifndef LI_SYMBOL_ssl_key
#define LI_SYMBOL_ssl_key
    LI_SYMBOL(ssl_key)
#endif

#ifndef LI_SYMBOL_update_secret_key
#define LI_SYMBOL_update_secret_key
    LI_SYMBOL(update_secret_key)
#endif

#ifndef LI_SYMBOL_user_id
#define LI_SYMBOL_user_id
    LI_SYMBOL(user_id)
#endif


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_SYMBOLS_HH



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

    auto res = li::tuple_reduce(metamap_values(where), stmt).template read_optional<O>();
    if (res)
      call_callback(s::read_access, *res, cb_args...);
    return res;
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
    stmt().map([&](const O& o) { f(o); });
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

