#pragma once

#include <sstream>

#include <li/json/decoder.hh>
#include <li/json/encoder.hh>
#include <li/json/utils.hh>

namespace li {

template <typename T> struct is_json_vector;

template <typename T> struct json_object_base {

public:
  inline auto downcast() const { return static_cast<const T*>(this); }

  template <typename C, typename O> void encode(C& output, O&& obj) const {
    return impl::json_encode(output, std::forward<O>(obj), *downcast());
  }

  template <typename C, typename... M> void encode(C& output, const metamap<M...>& obj) const {
    return impl::json_encode(output, obj, *downcast());
  }

  template <typename O> std::string encode(O obj) const {
    std::ostringstream ss;
    impl::json_encode(ss, std::forward<O>(obj), *downcast());
    return ss.str();
  }

  template <typename... M> std::string encode(const metamap<M...>& obj) const {
    std::ostringstream ss;
    impl::json_encode(ss, obj, *downcast());
    return ss.str();
  }

  template <typename C, typename G> void encode_generator(C& output, int size, G& generator) const {
    static_assert(is_json_vector<T>::value, "encode_generator is only supported on json vector");
      return this->encode(output, mmm(s::size = size, s::generator = generator));
  }
  template <typename G> std::string encode_generator(int size, G& generator) const {
    static_assert(is_json_vector<T>::value, "encode_generator is only supported on json vector");
      return this->encode(mmm(s::size = size, s::generator = generator));
  }


  template <typename C, typename O> json_error decode(C& input, O& obj) const {
    return impl::json_decode(input, obj, *downcast());
  }

  template <typename C, typename... M> auto decode(C& input) const {
    auto map = impl::json_object_to_metamap(*downcast());
    impl::json_decode(input, map, *downcast());
    return map;
  }
};

template <typename T> struct json_object_ : public json_object_base<json_object_<T>> {
  json_object_() = default;
  json_object_(const T& s) : schema(s) {}
  T schema;
};

template <typename... S> auto json_object(S&&... s) {
  auto members = std::make_tuple(impl::make_json_object_member(std::forward<S>(s))...);
  return json_object_<decltype(members)>{members};
}

template <typename V> struct json_value_ : public json_object_base<json_value_<V>> {
  json_value_() = default;
};

template <typename V> auto json_value(V&& v) { return json_value_<V>{}; }

template <typename T> struct json_vector_ : public json_object_base<json_vector_<T>> {
  enum { is_json_vector = true };
  json_vector_() = default;
  json_vector_(const T& s) : schema(s) {}
  T schema;
};

template <typename... S> auto json_object_vector(S&&... s) {
  auto obj = json_object(std::forward<S>(s)...);
  return json_vector_<decltype(obj)>{obj};
}
template <typename E> auto json_vector(E&& element) {
  return json_vector_<decltype(element)>{element};
}

template <typename T, unsigned N> struct json_static_array_ : public json_object_base<json_static_array_<T, N>> {
  enum { is_json_static_array = true };
  json_static_array_() = default;
  json_static_array_(const T& s) : element_schema(s) {}
  T element_schema;
};

template <unsigned N, typename... S> auto json_static_array(S&&... s) {
  auto obj = json_object(std::forward<S>(s)...);
  return json_static_array_<decltype(obj), N>{obj};
}

template <typename... T> struct json_tuple_ : public json_object_base<json_tuple_<T...>> {
  json_tuple_() = default;
  json_tuple_(const T&... s) : elements(s...) {}
  std::tuple<T...> elements;
};

template <typename... S> auto json_tuple(S&&... s) { return json_tuple_<S...>{s...}; }

struct json_key {
  inline json_key(const char* c) : key(c) {}
  const char* key;
};

template <typename V> struct json_map_ : public json_object_base<json_map_<V>> {
  json_map_() = default;
  json_map_(const V& s) : mapped_schema(s) {}
  V mapped_schema;
};

template <typename V> auto json_map() { return json_map_<V>(); }

template <typename... T> struct json_variant_ : public json_object_base<json_variant_<T...>> {
  json_variant_() = default;
  json_variant_(const T&... s) : elements(s...) {}
  std::tuple<T...> elements;
};

template <typename... S> auto json_variant(S&&... s) { return json_variant_<S...>(s...); }

template <typename C, typename M> decltype(auto) json_decode(C& input, M& obj) {
  return impl::to_json_schema(obj).decode(input, obj);
}

template <typename C, typename M> decltype(auto) json_encode(C& output, const M& obj) {
  impl::to_json_schema(obj).encode(output, obj);
}

template <typename M> auto json_encode(M&& obj) {
  return impl::to_json_schema(obj).encode(std::forward<M>(obj));
}

template <typename C, typename F> decltype(auto) json_encode_generator(C& output, int N, F generator) {
  auto elt = impl::to_json_schema(decltype(generator()){});
  json_vector_<decltype(elt)>(elt).encode_generator(output, N, generator);
}
template <typename F> decltype(auto) json_encode_generator(int N, F generator) {
  auto elt = impl::to_json_schema(decltype(generator()){});
  return json_vector_<decltype(elt)>(elt).encode_generator(N, generator);
}

template <typename A, typename B, typename... C>
auto json_encode(const assign_exp<A, B>& exp, C... c) {
  auto obj = mmm(exp, c...);
  return impl::to_json_schema(obj).encode(obj);
}

} // namespace li
