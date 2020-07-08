// Author: Matthieu Garrigues matthieu.garrigues@gmail.com
//
// Single header version the lithium_symbol library.
// https://github.com/matt-42/lithium
//
// This file is generated do not edit it.

#pragma once

#include <utility>
#include <vector>


#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SYMBOL_SYMBOL_HH
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SYMBOL_SYMBOL_HH

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

