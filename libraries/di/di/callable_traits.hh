#pragma once

#include <type_traits>

namespace li
{

  namespace internal
  {
    template <typename T>
    struct has_parenthesis_operator
    {
      template<typename C> 
      static char test(decltype(&C::operator()));
      template<typename C>
      static int test(...);
      static const bool value = sizeof(test<T>(0)) == 1;
    };

  }

  template <typename F, typename X = void>
  struct callable_traits
  {
    typedef std::false_type is_callable;
    static const int arity = 0;
    typedef std::tuple<> arguments_tuple;
    typedef typelist<> arguments_list;
    typedef void return_type;
  };

  template <typename F, typename X>
  struct callable_traits<F&, X> : public callable_traits<F, X> {};
  template <typename F, typename X>
  struct callable_traits<F&&, X> : public callable_traits<F, X> {};
  template <typename F, typename X>
  struct callable_traits<const F&, X> : public callable_traits<F, X> {};

  template <typename F>
  struct callable_traits<F, std::enable_if_t<internal::has_parenthesis_operator<F>::value>>
  {
    typedef callable_traits<decltype(&F::operator())> super;
    typedef std::true_type is_callable;
    static const int arity = super::arity;
    typedef typename super::arguments_tuple arguments_tuple;
    typedef typename super::arguments_list arguments_list;
    typedef typename super::return_type return_type;
  };

  template <typename C, typename R, typename... ARGS>
  struct callable_traits<R (C::*)(ARGS...) const>
  {
    typedef std::true_type is_callable;
    static const int arity = sizeof...(ARGS);
    typedef std::tuple<ARGS...> arguments_tuple;
    typedef typelist<ARGS...> arguments_list;
    typedef R return_type;
  };

  template <typename C, typename R, typename... ARGS>
  struct callable_traits<R (C::*)(ARGS...)>
  {
    typedef std::true_type is_callable;
    static const int arity = sizeof...(ARGS);
    typedef std::tuple<ARGS...> arguments_tuple;
    typedef typelist<ARGS...> arguments_list;
    typedef R return_type;
  };
  
  template <typename R, typename... ARGS>
  struct callable_traits<R(ARGS...)>
  {
    typedef std::true_type is_callable;
    static const int arity = sizeof...(ARGS);
    typedef std::tuple<ARGS...> arguments_tuple;
    typedef typelist<ARGS...> arguments_list;
    typedef R return_type;
  };

  
  template <typename R, typename... ARGS>
  struct callable_traits<R(*)(ARGS...)>
  {
    typedef std::true_type is_callable;
    static const int arity = sizeof...(ARGS);
    typedef std::tuple<ARGS...> arguments_tuple;
    typedef typelist<ARGS...> arguments_list;
    typedef R return_type;
  };
}
