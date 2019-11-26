#pragma once

#include <utility>
#include <iod/symbol/ast.hh>

namespace iod {

  template <typename S>
  class symbol : public assignable<S>,
                 public array_subscriptable<S>,
                 public callable<S>,
                 public Exp<S>
  {};
}

#define IOD_SYMBOL(NAME)                                                \
namespace s {                                                           \
struct NAME##_t : iod::symbol<NAME##_t> {                         \
                                                                        \
using assignable<NAME##_t>::operator=;                               \
                                                                        \
inline constexpr bool operator==(NAME##_t) { return true; }          \
  template <typename T>                                                 \
  inline constexpr bool operator==(T) { return false; }                 \
                                                                        \
template <typename V>                                                   \
  struct variable_t {                                                   \
    typedef NAME##_t _iod_symbol_type;                            \
    typedef V _iod_value_type;                                          \
    V NAME;                                                             \
  };                                                                   \
                                                                        \
  template <typename T, typename... A>                                  \
  static inline decltype(auto) symbol_method_call(T&& o, A... args) { return o.NAME(args...); } \
  template <typename T, typename... A>                                  \
  static inline auto& symbol_member_access(T&& o) { return o.NAME; } \
  template <typename T>                                                \
  static constexpr auto has_getter(int) -> decltype(std::declval<T>().NAME(), std::true_type{}) { return {}; } \
  template <typename T>                                                \
  static constexpr auto has_getter(long) { return std::false_type{}; }     \
  template <typename T>                                                \
  static constexpr auto has_member(int) -> decltype(std::declval<T>().NAME, std::true_type{}) { return {}; } \
  template <typename T>                                                \
  static constexpr auto has_member(long) { return std::false_type{}; }        \
                                                                        \
  static inline auto symbol_string()                                    \
  {                                                                     \
    return #NAME;                                                       \
  }                                                                     \
                                                                        \
};                                                                      \
static constexpr  NAME##_t NAME;                                    \
}


namespace iod {

  template <typename S>
  inline decltype(auto) make_variable(S s, char const v[])
  {
    typedef typename S::template variable_t<const char*> ret;
    return ret{v};
  }

  template <typename V, typename S>
  inline decltype(auto) make_variable(S s, V v)
  {
    typedef typename S::template variable_t<std::remove_const_t<std::remove_reference_t<V>>> ret;
    return ret{v};
  }
  
  template <typename K, typename V>
  inline decltype(auto) make_variable_reference(K s, V&& v)
  {
    typedef typename K::template variable_t<V> ret;
    return ret{v};
  }

  template <typename T, typename S, typename... A>
  static inline decltype(auto) symbol_method_call(T&& o, S, A... args)
  {
    return S::symbol_method_call(o, std::forward<A>(args)...);
  }

  template <typename T, typename S>
  static inline decltype(auto) symbol_member_access(T&& o, S)
  {
    return S::symbol_member_access(o);
  }
  
  template <typename T, typename S>
  constexpr auto has_member(T&& o, S) { return S::template has_member<T>(0); }
  template <typename T, typename S>
  constexpr auto has_member() { return S::template has_member<T>(0); }

  template <typename T, typename S>
  constexpr auto has_getter(T&& o, S) { return decltype(S::template has_getter<T>(0)){}; }
  template <typename T, typename S>
  constexpr auto has_getter() { return decltype(S::template has_getter<T>(0)){}; }
  
  template <typename S, typename T>
  struct CANNOT_FIND_REQUESTED_MEMBER_IN_TYPE {};
  
  template <typename T, typename S>
  decltype(auto) symbol_member_or_getter_access(T&&o, S)
  {
    if constexpr(has_getter<T, S>()) {
        return symbol_method_call(o, S{});
      }
    else if constexpr(has_member<T, S>()) {
        return symbol_member_access(o, S{});
      }
    else
    {
      return CANNOT_FIND_REQUESTED_MEMBER_IN_TYPE<S, T>::error;
    }
                   
  }
  
  template <typename S>
  auto symbol_string(symbol<S> v)
  {
    return S::symbol_string();
  }

  template <typename V>
  auto symbol_string(V v, typename V::_iod_symbol_type* = 0)
  {
    return V::_iod_symbol_type::symbol_string();
  }
}

