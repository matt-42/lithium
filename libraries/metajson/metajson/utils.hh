#pragma once

#include <optional>

#include <iod/metamap/metamap.hh>
#include <iod/metajson/symbols.hh>
#include <iod/symbol/ast.hh>
#include <tuple>

namespace iod {


  
  
  
  
  
  template <typename T>
  struct json_object_base;
  
  template <typename T>    struct json_object_;
  template <typename T>    struct json_vector_;
  template <typename V>    struct json_value_;
  template <typename... T> struct json_tuple_;
  struct json_key;
  
  namespace impl
  {
    template <typename S, typename... A>
    auto make_json_object_member(const function_call_exp<S, A...>& e);
    template <typename S>
    auto make_json_object_member(const iod::symbol<S>&);

    template <typename S, typename T>
    auto make_json_object_member(const assign_exp<S, T>& e)
    {
      return cat(make_json_object_member(e.left),
                 make_metamap(s::type = e.right));
    }

    template <typename S>
    auto make_json_object_member(const iod::symbol<S>&)
    {
      return make_metamap(s::name = S{});
    }

    template <typename V>
    auto to_json_schema(V v)
    {
      return json_value_<V>{};
    }

    template <typename... M>
    auto to_json_schema(const metamap<M...>& m);
    
    template <typename V>
    auto to_json_schema(const std::vector<V>& arr)
    {
      auto elt = to_json_schema(decltype(arr[0]){});
      return json_vector_<decltype(elt)>{elt};
    }
    
    template <typename... V>
    auto to_json_schema(const std::tuple<V...>& arr)
    {
      return json_tuple_<decltype(to_json_schema(V{}))...>(to_json_schema(V{})...);
    }

    template <typename... M>
    auto to_json_schema(const metamap<M...>& m)
    {
      auto tuple_maker = [] (auto&&... t) { return std::make_tuple(std::forward<decltype(t)>(t)...); };

      auto entities = map_reduce(m, [] (auto k, auto v) {
          return make_metamap(s::name = k, s::type = to_json_schema(v));
        }, tuple_maker);


      return json_object_<decltype(entities)>(entities);
    }

    
    template <typename... E>
    auto json_object_to_metamap(const json_object_<std::tuple<E...>>& s)
    {
      auto make_kvs = [] (auto... elt)
        {
          return std::make_tuple((elt.name = elt.type)...);
        };

      auto kvs = std::apply(make_kvs, s.entities);
      return std::apply(make_metamap, kvs);
    }

    template <typename S, typename... A>
    auto make_json_object_member(const function_call_exp<S, A...>& e)
    {
      auto res = make_metamap(s::name = e.method, s::json_key = symbol_string(e.method));

      auto parse = [&] (auto a)
        {
          if constexpr(std::is_same<decltype(a), json_key>::value) {
              res.json_key = a.key;
            }
        };

      ::iod::tuple_map(e.args, parse);
      return res;
    }
    
  }

  template <typename T>
  struct json_object_;

  template <typename O>
  struct json_vector_;
  

  template <typename E> constexpr auto json_is_vector(json_vector_<E>) ->  std::true_type { return {}; }
  template <typename E> constexpr auto json_is_vector(E) ->  std::false_type { return {}; }

  template <typename... E> constexpr auto json_is_tuple(json_tuple_<E...>) ->  std::true_type { return {}; }
  template <typename E> constexpr auto json_is_tuple(E) ->  std::false_type { return {}; }
  
  template <typename E> constexpr auto json_is_object(json_object_<E>) ->  std::true_type { return {}; }
  template <typename E> constexpr auto json_is_object(E) ->  std::false_type { return {}; }

  template <typename E> constexpr auto json_is_value(json_object_<E>) ->  std::false_type { return {}; }
  template <typename E> constexpr auto json_is_value(json_vector_<E>) ->  std::false_type { return {}; }
  template <typename... E> constexpr auto json_is_value(json_tuple_<E...>) ->  std::false_type { return {}; }
  template <typename E> constexpr auto json_is_value(E) ->  std::true_type { return {}; }


  template <typename T>
  constexpr auto is_std_optional(std::optional<T>) -> std::true_type;
  template <typename T>
  constexpr auto is_std_optional(T) -> std::false_type;
  
}
