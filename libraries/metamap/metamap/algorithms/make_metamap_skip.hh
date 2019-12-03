#pragma once

#include <li/metamap/metamap.hh>
#include <li/metamap/algorithms/cat.hh>

namespace li {

  
  struct skip {};
  static struct {

    template <typename... M, typename ...T>
    inline decltype(auto) run(metamap<M...> map, skip, T&&... args) const
    {
      return run(map, std::forward<T>(args)...);
    }
    
    template <typename T1, typename... M, typename ...T>
    inline decltype(auto) run(metamap<M...> map, T1&& a, T&&... args) const
    {
      return run(cat(map,
                     internal::make_metamap_helper(internal::exp_to_variable(std::forward<T1>(a)))),
                 std::forward<T>(args)...);
    }

    template <typename... M>
    inline decltype(auto) run(metamap<M...> map) const { return map; }
    
    template <typename... T>
    inline decltype(auto) operator()(T&&... args) const
    {
      // Copy values.
      return run(metamap<>{}, std::forward<T>(args)...);
    }

  } make_metamap_skip;

}
