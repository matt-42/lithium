#pragma once

#include <optional>

#include <li/json/symbols.hh>
#include <li/metamap/metamap.hh>
#include <li/symbol/ast.hh>
#include <tuple>
#include <map>
#include <unordered_map>
#include <variant>

namespace li {

template <typename T> struct json_object_base;

template <typename T> struct json_object_;
template <typename T> struct json_vector_;
template <typename T, unsigned N> struct json_static_array_;
template <typename V> struct json_value_;
template <typename V> struct json_map_;
template <typename... T> struct json_tuple_;
template <typename... T> struct json_variant_;
struct json_key;

namespace impl {
template <typename S, typename... A>
auto make_json_object_member(const function_call_exp<S, A...>& e);
template <typename S> auto make_json_object_member(const li::symbol<S>&);

template <typename S, typename T> auto make_json_object_member(const assign_exp<S, T>& e) {
  return cat(make_json_object_member(e.left), mmm(s::type = e.right));
}

template <typename S> auto make_json_object_member(const li::symbol<S>&) {
  return mmm(s::name = S{});
}

template <typename V> auto to_json_schema(V v);
template <typename... M> auto to_json_schema(const metamap<M...>& m);
template <typename V> auto to_json_schema(const std::vector<V>& arr);
template <typename... V> auto to_json_schema(const std::tuple<V...>& arr);
template <typename... V> auto to_json_schema(const std::variant<V...>& v);
template <typename K, typename V> auto to_json_schema(const std::unordered_map<K, V>& arr);
template <typename K, typename V> auto to_json_schema(const std::map<K, V>& arr);
template <typename... M> auto to_json_schema(const metamap<M...>& m);
template <typename V, unsigned N> auto to_json_schema(const V (&v)[N]);
template <unsigned N> auto to_json_schema(const char (&v)[N]);

template <typename V> auto to_json_schema(V v) {
  if constexpr (std::is_pointer_v<V> and !std::is_same_v<const char*, V> and !std::is_array_v<V>)
    return to_json_schema(*v);
  else
   return json_value_<V>{}; 
}

template <typename V> auto to_json_schema(const std::vector<V>& arr) {
  auto elt = to_json_schema(V{});
  return json_vector_<decltype(elt)>{elt};
}
template <unsigned N> auto to_json_schema(const char (&v)[N]) {
  return json_value_<const char[N]>{};
}

template <typename... V> auto to_json_schema(const std::tuple<V...>& arr) {
  return json_tuple_<decltype(to_json_schema(V{}))...>(to_json_schema(V{})...);
}
template <typename... V> auto to_json_schema(const std::variant<V...>& v) {
  return json_variant_<decltype(to_json_schema(V{}))...>(to_json_schema(V{})...);
}
template <typename V, unsigned N> auto to_json_schema(const V (&v)[N]) {
  auto elt = to_json_schema(V{});
  return json_static_array_<decltype(elt), N>(elt);
}
template <typename K, typename V> auto to_json_schema(const std::unordered_map<K, V>& arr) {
  return json_map_<decltype(to_json_schema(V{}))>(to_json_schema(V{}));
}
template <typename K, typename V> auto to_json_schema(const std::map<K, V>& arr) {
  return json_map_<decltype(to_json_schema(V{}))>(to_json_schema(V{}));
}

template <typename... M> auto to_json_schema(const metamap<M...>& m) {
  auto tuple_maker = [](auto&&... t) { return std::make_tuple(std::forward<decltype(t)>(t)...); };

  auto entities = map_reduce(
      m, [](auto k, auto v) { return mmm(s::name = k, s::type = to_json_schema(v)); }, tuple_maker);

  return json_object_<decltype(entities)>(entities);
}


template <typename... E> auto json_object_to_metamap(const json_object_<std::tuple<E...>>& s) {
  auto make_mmm = [](auto... elt) { return mmm((elt.name = elt.type)...); };
  return std::apply(make_mmm, s.entities);
}

template <typename S, typename... A>
auto make_json_object_member(const function_call_exp<S, A...>& e) {
  auto res = mmm(s::name = e.method, s::json_key = symbol_string(e.method));

  auto parse = [&](auto a) {
    if constexpr (std::is_same<decltype(a), json_key>::value) {
      res.json_key = a.key;
    }
  };

  ::li::tuple_map(e.args, parse);
  return res;
}

} // namespace impl

template <typename T> struct json_object_;

template <typename O> struct json_vector_;

template <typename T> struct is_json_vector : std::false_type {};
template <typename T> struct is_json_vector<json_vector_<T>> : std::true_type {};

template <typename E> constexpr auto json_is_vector(json_vector_<E>) -> std::true_type {
  return {};
}
template <typename E> constexpr auto json_is_vector(E) -> std::false_type { return {}; }
template <typename E, unsigned N> constexpr auto json_is_static_array(json_static_array_<E, N>) -> std::true_type {
  return {};
}
template <typename E> constexpr auto json_is_static_array(E) -> std::false_type { return {}; }

template <typename... E> constexpr auto json_is_tuple(json_tuple_<E...>) -> std::true_type {
  return {};
}
template <typename E> constexpr auto json_is_tuple(E) -> std::false_type { return {}; }

template <typename... E> constexpr auto json_is_variant(json_variant_<E...>) -> std::true_type {
  return {};
}
template <typename E> constexpr auto json_is_variant(E) -> std::false_type { return {}; }

template <typename E> constexpr auto json_is_object(json_object_<E>) -> std::true_type {
  return {};
}
template <typename E> constexpr auto json_is_object(json_map_<E>) -> std::true_type {
  return {};
}
template <typename E> constexpr auto json_is_object(E) -> std::false_type { return {}; }


template <typename E> constexpr auto json_is_value(json_object_<E>) -> std::false_type {
  return {};
}
template <typename... E> constexpr auto json_is_value(json_variant_<E...>) -> std::false_type {
  return {};
}
template <typename E> constexpr auto json_is_value(json_vector_<E>) -> std::false_type {
  return {};
}
template <typename... E> constexpr auto json_is_value(json_tuple_<E...>) -> std::false_type {
  return {};
}
template <typename E> constexpr auto json_is_value(json_map_<E>) -> std::false_type {
  return {};
}
template <typename E> constexpr auto json_is_value(E) -> std::true_type { return {}; }

template <typename T> constexpr auto is_std_optional(std::optional<T>) -> std::true_type;
template <typename T> constexpr auto is_std_optional(T) -> std::false_type;

} // namespace li
