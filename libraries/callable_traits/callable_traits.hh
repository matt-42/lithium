#pragma once

namespace li {

  template <typename ...T>
  struct typelist {};

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

  // Traits on callable (function, functors and lambda functions).

  // callable_traits<F>::is_callable = true_type if F is callable.
  // callable_traits<F>::arity = N if F takes N arguments.
  // callable_traits<F>::arguments_tuple_type = tuple<Arg1, ..., ArgN>

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

  template <typename F>
  using callable_arguments_tuple_t = typename callable_traits<F>::arguments_tuple;
  template <typename F>
  using callable_arguments_list_t = typename callable_traits<F>::arguments_list;
  template <typename F>
  using callable_return_type_t = typename callable_traits<F>::return_type;

  template <typename F>
  struct is_callable : public callable_traits<F>::is_callable {};
  
  template <typename F, typename... A>
  struct callable_with
  {
    template<typename G, typename... B> 
    static char test(int x, std::remove_reference_t<decltype(std::declval<G>()(std::declval<B>()...))>* = 0);
    template<typename G, typename... B> 
    static int test(...);
    static const bool value = sizeof(test<F, A...>(0)) == 1;    
  };

}
