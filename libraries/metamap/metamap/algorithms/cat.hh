#pragma once

#include <iod/metamap/metamap.hh>

namespace iod {


  template <typename ...T, typename ...U>
  inline decltype(auto) cat(const metamap<T...>& a,
                            const metamap<U...>& b)
  {
    return metamap<T..., U...>(*static_cast<const T*>(&a)...,
                               *static_cast<const U*>(&b)...);
  }
  
}
