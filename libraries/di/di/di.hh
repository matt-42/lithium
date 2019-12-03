#pragma once

#include <li/metamap/metamap.hh>
#include <li/di/callable_traits.hh>

namespace li
{

  namespace impl
  {
    template <typename T, std::size_t I>
    struct smap
    {};
      
#define LI_SMAP_ELT(I)                                                 \
    namespace smap_symbols { LI_SYMBOL_BODY(s##I) }                   \
    template <typename T>                                               \
    struct smap<T, I>                                                   \
    {                                                                   \
      auto get_symbol2(std::remove_reference_t<T>) const { return smap_symbols::s##I; }; \
    };

    LI_SMAP_ELT(0) LI_SMAP_ELT(1) LI_SMAP_ELT(2) LI_SMAP_ELT(3) LI_SMAP_ELT(4)
    LI_SMAP_ELT(5) LI_SMAP_ELT(6) LI_SMAP_ELT(7) LI_SMAP_ELT(8) LI_SMAP_ELT(9)
    LI_SMAP_ELT(10) LI_SMAP_ELT(11) LI_SMAP_ELT(12) LI_SMAP_ELT(13) LI_SMAP_ELT(14)
    LI_SMAP_ELT(15) LI_SMAP_ELT(16) LI_SMAP_ELT(17) LI_SMAP_ELT(18) LI_SMAP_ELT(19)
    LI_SMAP_ELT(20) LI_SMAP_ELT(21) LI_SMAP_ELT(22) LI_SMAP_ELT(23) LI_SMAP_ELT(24)

    template <typename... T>
    struct type_to_symbol;

    template <typename C>
    struct DI_CALL_MISSING_OR_DUPLICATE_ARGUMENT_OF_TYPE {};

    template <std::size_t... I, typename... T>
    struct type_to_symbol<std::index_sequence<I...>, T...> : public smap<T, I>...
    {
      using smap<T, I>::get_symbol2...;

      template <typename V>
      constexpr auto provide_type(V) const -> decltype(this->get_symbol2(std::declval<V>()), std::true_type{})
      { return {}; }

      constexpr auto provide_type(...) const -> std::false_type
      {
        return {};
      }

      template <typename V>
        auto get_symbol() const {

        if constexpr(decltype(this->template provide_type(V{})){}) {
            return decltype(this->get_symbol2(std::declval<V>())){};
          }
        else
        {
          DI_CALL_MISSING_OR_DUPLICATE_ARGUMENT_OF_TYPE<V>::error;
        }
      }

    };

    template <typename... T>
    struct type_to_symbol : type_to_symbol<std::make_index_sequence<sizeof...(T)>, T...>
    {};

    template <typename A, typename M, typename F, typename S>
    decltype(auto) retrieve_arg(M&& arg_map, F&& factory_map, S&& symbol_map)
    {
      auto key = symbol_map.template get_symbol<A>();
      if constexpr(decltype(has_key(arg_map, key)){}) {
          return arg_map[key];
        }
      // else if constexpr(has_static_make_method<A>()) {
      //     return di_call_recurse(&V::make, arg_map, factory_map);
      //     return arg_map[key];
      //   }
      // else if constexpr(has_key(factory_map, key)) {
      //     auto factory = factory_map[key];
      //     typedef decltype(factory) FT;
      //     return di_call_method_recurse(&FT::make, factory, arg_map, factory_map);
      //   }
      else
      {
        DI_CALL_MISSING_OR_DUPLICATE_ARGUMENT_OF_TYPE<A>::please_provide_an_argurment_of_this_type_or_a_factory_to_make_it;
      }
    }


    
    template <typename B>
    constexpr auto has_make_method(B) -> decltype((std::result_of_t<decltype(B::make)>*)42, std::true_type{})  {return {}; };
    constexpr auto has_make_method(...) -> std::false_type {return {};};

    template <typename B>
    auto di_factory_return_type()
    {
      return *(B*) 42;
      // if constexpr(has_make_method((B*)0)) return *(std::result_of_t<decltype(B::make)>*) 42;
      // else return *(B*) 42;
    }
    
    template <typename F, typename... A, typename... B, std::size_t... I>
    decltype(auto) di_call2(F fun, typelist<A...>,
                            B&&... to_inject)
    {
      impl::type_to_symbol<B...> symbol_map;

      auto arg_map = make_metamap_reference((symbol_map.template get_symbol<B>() = std::forward<B>(to_inject))...);
      // auto factory_map = make_metamap_reference((symbol_map.template get_symbol<decltype(di_factory_return_type<B>())> = std::forward<B>(to_inject))...);

      auto factory_map = arg_map;
      //return fun(arg_map[symbol_map.template get_symbol<A>()]...);
      return fun(retrieve_arg<A>(arg_map, factory_map, symbol_map)...);
    }
  

  }
  
  template <typename F, typename... B>
  decltype(auto) di_call(F fun, B&&... to_inject)
  {
    return impl::di_call2(fun, typename callable_traits<F>::arguments_list{},
                          std::forward<B>(to_inject)...);
  }
}
