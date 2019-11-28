#pragma once

#include <utility>
#include <tuple>

namespace iod {

  namespace internal
  {
    struct {
      template <typename A, typename... B>
      constexpr auto operator()(A&& a, B&&... b)
      {
        auto result = a;
        using expand_variadic_pack  = int[];
        (void)expand_variadic_pack{0, ((result += b), 0)... };
        return result;
      }
    } reduce_add;

  }

  
  template <typename... Ms>
  struct metamap;
  
  template <typename F, typename... M>
  decltype(auto) find_first(metamap<M...>&& map, F fun);
  
  template <typename ...Ms>
  struct metamap : public Ms...
  {
    typedef metamap<Ms...> self;
    // Constructors.
    inline metamap() = default;
    inline metamap(self&&) = default;
    inline metamap(const self&) = default;
    self& operator=(const self&) = default;

    metamap(self& other)
      : metamap(const_cast<const self&>(other)) {}

    template <typename... M>
    inline metamap(M&&... members) : Ms(std::forward<M>(members))... {}

    // Assignemnt ?

    // Retrive a value.
    template <typename K>
    decltype(auto) operator[](K k)
    {
      return symbol_member_access(*this, k);
    }

    template <typename K>
    decltype(auto) operator[](K k) const
    {
      return symbol_member_access(*this, k);
    }

  };

  template <typename... Ms>
  constexpr auto size(metamap<Ms...>)
  {
    return sizeof...(Ms);
  }
  
  template <typename M>
  struct metamap_size_t
  {
  };
  template <typename... Ms>
  struct metamap_size_t<metamap<Ms...>>
  {
    enum { value = sizeof...(Ms) };
  };
  template <typename M>
  constexpr int metamap_size()
  {
    return metamap_size_t<std::decay_t<M>>::value;
  }


  template <typename... Ks>
  decltype(auto) metamap_values(const metamap<Ks...>& map)
  {
    return std::forward_as_tuple(map[typename Ks::_iod_symbol_type()]...);
  }

  template <typename K, typename M>
  constexpr auto has_key(M&& map, K k)
  {
    return decltype(has_member(map, k)){};
  }

  template <typename M, typename K>
  constexpr auto has_key(K k)
  {
    return decltype(has_member(std::declval<M>(), k)){};
  }

  template <typename K, typename M, typename O>
  constexpr auto get_or(M&& map, K k, O default_)
  {
    if constexpr(has_key<M>(k)) {
        return map[k];
      }
    else
      return default_;
  }
  
  template <typename X>
  struct is_metamap { enum { ret = false }; };
  template <typename... M>
  struct is_metamap<metamap<M...>> { enum { ret = true }; };
  
}

#include <iod/metamap/make.hh>
#include <iod/metamap/algorithms/map_reduce.hh>
#include <iod/metamap/algorithms/intersection.hh>
#include <iod/metamap/algorithms/substract.hh>
#include <iod/metamap/algorithms/cat.hh>
#include <iod/metamap/algorithms/tuple_utils.hh>
