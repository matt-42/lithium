#pragma once

#include <li/metamap/algorithms/make_metamap_skip.hh>
#include <li/metamap/algorithms/map_reduce.hh>

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
