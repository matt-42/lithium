#pragma once

#include <li/json/symbols.hh>
#include <li/json/unicode.hh>
#include <li/json/utils.hh>
#include <li/metamap/metamap.hh>
#include <li/symbol/symbol.hh>
#include <string_view>
#include <tuple>
#include <variant>

namespace li {

using std::string_view;

template <typename... T> struct json_tuple_;
template <typename T> struct json_object_;

namespace impl {

// Json encoder.
// =============================================

template <typename C, typename O, typename E>
inline void json_encode(C& ss, O obj, const json_object_<E>& schema);
template <typename C, typename... E, typename... T>
inline void json_encode(C& ss, const std::tuple<T...>& tu, const json_tuple_<E...>& schema);
template <typename T, typename C, typename E>
inline void json_encode(C& ss, const T& value, const E& schema);
template <typename T, typename C, typename E>
inline void json_encode(C& ss, const std::vector<T>& array, const json_vector_<E>& schema);

template <typename T, typename C> inline void json_encode_value(C& ss, const T& t) { ss << t; }

template <typename C> inline void json_encode_value(C& ss, const char* s) { 
  //ss << s;
  utf8_to_json(s, ss);
 }

template <typename C> inline void json_encode_value(C& ss, const string_view& s) {
  //ss << s;
  utf8_to_json(s, ss);
}

template <typename C> inline void json_encode_value(C& ss, const std::string& s) {
  //ss << s;
  utf8_to_json(s, ss);
}

template <typename C, typename... T> inline void json_encode_value(C& ss, const metamap<T...>& s) {
  json_encode(ss, s, to_json_schema(s));
}

template <typename T, typename C> inline void json_encode_value(C& ss, const std::optional<T>& t) {
  if (t.has_value())
    json_encode_value(ss, t.value());
}

template <typename C, typename... T>
inline void json_encode_value(C& ss, const std::variant<T...>& t) {
  ss << "{\"idx\":" << t.index() << ",\"value\":";
  std::visit([&](auto&& value) { json_encode_value(ss, value); }, t);
  ss << '}';
}

template <typename C, typename O, typename E>
inline void json_encode(C& ss, O obj, const json_object_<E>& schema);

template <typename T, typename C, typename E>
inline void json_encode(C& ss, const T& value, const E& schema) {
  json_encode_value(ss, value);
}

template <typename T, typename C, typename E>
inline void json_encode(C& ss, const std::vector<T>& array, const json_vector_<E>& schema) {
  ss << '[';
  for (const auto& t : array) {
    if constexpr (decltype(json_is_vector(E{})){} or decltype(json_is_object(E{})){}) {
      json_encode(ss, t, schema.schema);
    } else
      json_encode_value(ss, t);

    if (&t != &array.back())
      ss << ',';
  }
  ss << ']';
}

template <typename V, typename C, typename M>
inline void json_encode(C& ss, const M& map, const json_map_<V>& schema) {
  ss << '{';
  bool first = true;
  for (const auto& pair : map) {
    if (!first)
      ss << ',';

    json_encode_value(ss, pair.first);
    ss << ':';

    if constexpr (decltype(json_is_value(schema.mapped_schema)){})
      json_encode_value(ss, pair.second);
    else
      json_encode(ss, pair.second, schema.mapped_schema);

    first = false;
  }

  ss << '}';
}

template <typename F, typename... E, typename... T, std::size_t... I>
inline void json_encode_tuple_elements(F& encode_fun, const std::tuple<T...>& tu,
                                       const std::tuple<E...>& schema, std::index_sequence<I...>) {
  (void)std::initializer_list<int>{((void)encode_fun(std::get<I>(tu), std::get<I>(schema)), 0)...};
}

template <typename C, typename... E, typename... T>
inline void json_encode(C& ss, const std::tuple<T...>& tu, const json_tuple_<E...>& schema) {
  ss << '[';
  bool first = true;
  auto encode_one_element = [&first, &ss](auto value, auto value_schema) {
    if (!first)
      ss << ',';
    first = false;
    if constexpr (decltype(json_is_value(value_schema)){}) {
      json_encode_value(ss, value);
    } else
      json_encode(ss, value, value_schema);
  };

  json_encode_tuple_elements(encode_one_element, tu, schema.elements,
                             std::make_index_sequence<sizeof...(T)>{});
  ss << ']';
}

template <typename C, typename O, typename E>
inline void json_encode(C& ss, O obj, const json_object_<E>& schema) {
  ss << '{';
  bool first = true;

  auto encode_one_entity = [&](auto e) {
    if constexpr (decltype(is_std_optional(symbol_member_or_getter_access(obj, e.name))){}) {
      if (!symbol_member_or_getter_access(obj, e.name).has_value())
        return;
    }

    if (!first) {
      ss << ',';
    }
    first = false;
    if constexpr (has_key(e, s::json_key)) {
      json_encode_value(ss, e.json_key);
    } else
      json_encode_value(ss, symbol_string(e.name));
    ss << ':';

    if constexpr (has_key(e, s::type)) {
      if constexpr (decltype(json_is_vector(e.type)){} or decltype(json_is_object(e.type)){}) {
        return json_encode(ss, symbol_member_or_getter_access(obj, e.name), e.type);
      } else
        json_encode_value(ss, symbol_member_or_getter_access(obj, e.name));
    } else
      json_encode_value(ss, symbol_member_or_getter_access(obj, e.name));
  };

  tuple_map(schema.schema, encode_one_entity);
  ss << '}';
}
} // namespace impl

} // namespace li
