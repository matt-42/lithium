#pragma once

#include <iod/metamap/metamap.hh>
#include <iod/metamap/algorithms/map_reduce.hh>
#include <iod/metamap/algorithms/make_metamap_skip.hh>

namespace iod {

  template <typename ...T, typename ...U>
  inline auto substract(const metamap<T...>& a,
                        const metamap<U...>& b)
  {
    return map_reduce(a, [&] (auto k, auto&& v) {
        if constexpr(!has_key<metamap<U...>>(k)) {
            return k = std::forward<decltype(v)>(v);
          }
        else return skip{}; }, make_metamap_skip);
  }

}
