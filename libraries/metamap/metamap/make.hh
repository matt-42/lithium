#pragma once

#include <li/symbol/ast.hh>
#include <li/symbol/symbol.hh>
#include <li/callable_traits/typelist.hh>

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

namespace internal {
  
  template <typename... V>
  auto make_metamap_type(typelist<V...> variables) {
    return mmm(V(typename V::left_t{}, typename V::right_t{})...);
  };

  template <typename T1, typename T2, typename... V, typename... T>
  auto make_metamap_type(typelist<V...> variables, T1, T2, T... args) {
    return make_metamap_type(typelist<V..., assign_exp<T1, T2>>{},
              args...);
  };
}

// Helper to make a metamap type:
//  metamap_t<s::name_t, string, s::age_t, int>
//  instead of decltype(mmm(s::name = string(), s::age = int()));
template <typename... T>
using metamap_t = decltype(internal::make_metamap_type(typelist<>{}, T{}...));


} // namespace li
