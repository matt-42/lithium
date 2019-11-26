#pragma once

#include <iod/metamap/metamap.hh>


namespace iod {

  // Map a function(key, value) on all kv pair
  template <typename... M, typename F>
  void map(const metamap<M...>& m, F fun)
  {
    auto apply = [&] (auto key) -> decltype(auto)
      {
        return fun(key, m[key]);
      };

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
  template <typename... M, typename F>
  void map(metamap<M...>& m, F fun)
  {
    auto apply = [&] (auto key) -> decltype(auto)
      {
        return fun(key, m[key]);
      };

    apply_each(apply, typename M::_iod_symbol_type{}...);
  }

  template <typename... E, typename F, typename R>
  auto apply_each2(F&& f, R&& r, E&&... e)
  {
    return r(f(std::forward<E>(e))...);
    //(void)std::initializer_list<int>{
    //  ((void)f(std::forward<E>(e)), 0)...};
  }

  // Map a function(key, value) on all kv pair an reduce
  // all the results value with the reduce(r1, r2, ...) function.
  template <typename... M, typename F, typename R>
  decltype(auto) map_reduce(const metamap<M...>& m, F map, R reduce)
  {
    auto apply = [&] (auto key) -> decltype(auto)
      {
        //return map(key, std::forward<decltype(m[key])>(m[key]));
        return map(key, m[key]);
      };

    return apply_each2(apply, reduce, typename M::_iod_symbol_type{}...);
    //return reduce(apply(typename M::_iod_symbol_type{})...);
  }

  // Map a function(key, value) on all kv pair an reduce
  // all the results value with the reduce(r1, r2, ...) function.
  template <typename... M, typename R>
  decltype(auto) reduce(const metamap<M...>& m, R reduce)
  {
    auto apply = [&] (auto key) -> decltype(auto)
      {
        return m[key];
      };

    return reduce(apply(typename M::_iod_symbol_type{})...);
  }

}
