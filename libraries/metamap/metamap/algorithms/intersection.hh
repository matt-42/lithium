#pragma once

#include <li/metamap/metamap.hh>
#include <li/metamap/algorithms/map_reduce.hh>
#include <li/metamap/algorithms/make_metamap_skip.hh>

namespace li {

  template <typename ...T, typename ...U>
  inline decltype(auto) intersection(const metamap<T...>& a,
                           const metamap<U...>& b)
  {
    return map_reduce(a, [&] (auto k, auto&& v) -> decltype(auto) {
        if constexpr(has_key<metamap<U...>, decltype(k)>()) {
            return k = std::forward<decltype(v)>(v);
          }
        else return skip{}; }, make_metamap_skip);
  }

}
