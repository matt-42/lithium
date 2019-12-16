// Author: Matthieu Garrigues matthieu.garrigues@gmail.com
//
// Single header version the lithium_http_backend library.
// https://github.com/matt-42/lithium
//
// This file is generated do not edit it.

#pragma once

#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <cstring>
#include <boost/lexical_cast.hpp>
#include <string.h>
#include <sys/sendfile.h>
#include <tuple>
#include <string_view>
#include <sys/mman.h>
#include <iostream>
#include <sys/epoll.h>
#include <variant>
#include <set>
#include <netinet/tcp.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <random>
#include <cassert>
#include <vector>
#include <thread>
#include <sys/uio.h>
#include <signal.h>
#include <cmath>
#include <utility>
#include <optional>
#include <unordered_map>
#include <map>
#include <mutex>
#include <memory>
#include <boost/context/continuation.hpp>
#include <string>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdio.h>
#include <functional>
#include <sys/stat.h>

#if defined(_MSC_VER)
#include <io.h>
#include <ciso646>
#endif // _MSC_VER


#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_HTTP_BACKEND
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_HTTP_BACKEND

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_JSON
#define LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_JSON


#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_DECODER
#define LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_DECODER

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_DECODE_STRINGSTREAM
#define LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_DECODE_STRINGSTREAM

#if defined(_MSC_VER)
#endif


namespace li {

using std::string_view;

namespace internal {
template <typename I> void parse_uint(I* val_, const char* str, const char** end) {
  I& val = *val_;
  val = 0;
  int i = 0;
  while (i < 40) {
    char c = *str;
    if (c < '0' or c > '9')
      break;
    val = val * 10 + c - '0';
    str++;
    i++;
  }
  if (end)
    *end = str;
}

template <typename I> void parse_int(I* val, const char* str, const char** end) {
  bool neg = false;

  if (str[0] == '-') {
    neg = true;
    str++;
  }
  parse_uint(val, str, end);
  if constexpr (!std::is_same<I, bool>::value) {
    if (neg)
      *val = -(*val);
  }
}

inline unsigned long long pow10(unsigned int e) {
  unsigned long long pows[] = {1,
                               10,
                               100,
                               1000,
                               10000,
                               100000,
                               1000000,
                               10000000,
                               100000000,
                               1000000000,
                               10000000000,
                               100000000000,
                               1000000000000,
                               10000000000000,
                               100000000000000,
                               1000000000000000,
                               10000000000000000,
                               100000000000000000};

  if (e < 18)
    return pows[e];
  else
    return 0;
}

template <typename F> void parse_float(F* f, const char* str, const char** end) {
  // 1.234e-10
  // [sign][int][decimal_part][exp]

  const char* it = str;
  int integer_part;
  parse_int(&integer_part, it, &it);
  int sign = integer_part >= 0 ? 1 : -1;
  *f = integer_part;
  if (*it == '.') {
    it++;
    unsigned long long decimal_part;
    const char* dec_end;
    parse_uint(&decimal_part, it, &dec_end);

    if (dec_end > it)
      *f += (F(decimal_part) / pow10(dec_end - it)) * sign;

    it = dec_end;
  }

  if (*it == 'e' || *it == 'E') {
    it++;
    bool neg = false;
    if (*it == '-') {
      neg = true;
      it++;
    }

    unsigned int exp = 0;
    parse_uint(&exp, it, &it);
    if (neg)
      *f = *f / pow10(exp);
    else
      *f = *f * pow10(exp);
  }

  if (end)
    *end = it;
}

} // namespace internal

class decode_stringstream {
public:
  inline decode_stringstream(string_view buffer_)
      : cur(buffer_.data()), bad_(false), buffer(buffer_) {}

  inline bool eof() const { return cur >= &buffer.back(); }
  inline const char peek() const { return *cur; }
  inline int get() { return *(cur++); }
  inline int bad() const { return bad_; }

  template <typename T> void operator>>(T& value) {
    eat_spaces();
    if constexpr (std::is_floating_point<T>::value) {
      // Decode floating point.
      eat_spaces();
      const char* end = nullptr;
      internal::parse_float(&value, cur, &end);
      if (end == cur)
        bad_ = true;
      cur = end;
    } else if constexpr (std::is_integral<T>::value) {
      // Decode integer.
      const char* end = nullptr;
      internal::parse_int(&value, cur, &end);
      if (end == cur)
        bad_ = true;
      cur = end;
    } else if constexpr (std::is_same<T, std::string>::value) {
      // Decode UTF8 string.
      json_to_utf8(*this, value);
    } else if constexpr (std::is_same<T, string_view>::value) {
      // Decoding to stringview does not decode utf8.

      if (get() != '"') {
        bad_ = true;
        return;
      }

      const char* start = cur;
      bool escaped = false;

      while (peek() != '\0' and (escaped or peek() != '"')) {
        int nb = 0;
        while (peek() == '\\')
          nb++;

        escaped = nb % 2;
        cur++;
      }
      const char* end = cur;
      value = string_view(start, end - start);

      if (get() != '"') {
        bad_ = true;
        return;
      }
    }
  }

private:
  inline void eat_spaces() {
    while (peek() < 33)
      ++cur;
  }

  int bad_;
  const char* cur;
  string_view buffer; //
};

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_DECODE_STRINGSTREAM

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_ERROR
#define LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_ERROR


namespace li {

enum json_error_code { JSON_OK = 0, JSON_KO = 1 };

struct json_error {
  json_error& operator=(const json_error&) = default;
  operator bool() { return code != 0; }
  bool good() { return code == 0; }
  bool bad() { return code != 0; }
  int code;
  std::string what;
};

int make_json_error(const char* what) { return 1; }
int json_no_error() { return 0; }

static int json_ok = json_no_error();

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_ERROR

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_UNICODE
#define LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_UNICODE



#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_METAMAP
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_METAMAP


namespace li {

namespace internal {
struct {
  template <typename A, typename... B> constexpr auto operator()(A&& a, B&&... b) {
    auto result = a;
    using expand_variadic_pack = int[];
    (void)expand_variadic_pack{0, ((result += b), 0)...};
    return result;
  }
} reduce_add;

} // namespace internal

template <typename... Ms> struct metamap;

template <typename F, typename... M> decltype(auto) find_first(metamap<M...>&& map, F fun);

template <typename... Ms> struct metamap;

template <typename M1, typename... Ms> struct metamap<M1, Ms...> : public M1, public Ms... {
  typedef metamap<M1, Ms...> self;
  // Constructors.
  inline metamap() = default;
  // inline metamap(self&&) = default;
  inline metamap(const self&) = default;
  self& operator=(const self&) = default;

  // metamap(self& other)
  //  : metamap(const_cast<const self&>(other)) {}

  inline metamap(typename M1::_iod_value_type&& m1, typename Ms::_iod_value_type&&... members) : M1{m1}, Ms{std::forward<typename Ms::_iod_value_type>(members)}... {}
  inline metamap(M1&& m1, Ms&&... members) : M1(m1), Ms(std::forward<Ms>(members))... {}
  inline metamap(const M1& m1, const Ms&... members) : M1(m1), Ms((members))... {}

  // Assignemnt ?

  // Retrive a value.
  template <typename K> decltype(auto) operator[](K k) { return symbol_member_access(*this, k); }

  template <typename K> decltype(auto) operator[](K k) const {
    return symbol_member_access(*this, k);
  }
};

template <> struct metamap<> {
  typedef metamap<> self;
  // Constructors.
  inline metamap() = default;
  // inline metamap(self&&) = default;
  inline metamap(const self&) = default;
  // self& operator=(const self&) = default;

  // metamap(self& other)
  //  : metamap(const_cast<const self&>(other)) {}

  // Assignemnt ?

  // Retrive a value.
  template <typename K> decltype(auto) operator[](K k) { return symbol_member_access(*this, k); }

  template <typename K> decltype(auto) operator[](K k) const {
    return symbol_member_access(*this, k);
  }
};

template <typename... Ms> constexpr auto size(metamap<Ms...>) { return sizeof...(Ms); }

template <typename M> struct metamap_size_t {};
template <typename... Ms> struct metamap_size_t<metamap<Ms...>> {
  enum { value = sizeof...(Ms) };
};
template <typename M> constexpr int metamap_size() {
  return metamap_size_t<std::decay_t<M>>::value;
}

template <typename... Ks> decltype(auto) metamap_values(const metamap<Ks...>& map) {
  return std::forward_as_tuple(map[typename Ks::_iod_symbol_type()]...);
}

template <typename K, typename M> constexpr auto has_key(M&& map, K k) {
  return decltype(has_member(map, k)){};
}

template <typename M, typename K> constexpr auto has_key(K k) {
  return decltype(has_member(std::declval<M>(), std::declval<K>())){};
}

template <typename M, typename K> constexpr auto has_key() {
  return decltype(has_member(std::declval<M>(), std::declval<K>())){};
}

template <typename K, typename M, typename O> constexpr auto get_or(M&& map, K k, O default_) {
  if constexpr (has_key<M, decltype(k)>()) {
    return map[k];
  } else
    return default_;
}

template <typename X> struct is_metamap {
  enum { ret = false };
};
template <typename... M> struct is_metamap<metamap<M...>> {
  enum { ret = true };
};

} // namespace li

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_CAT
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_CAT

namespace li {

template <typename... T, typename... U>
inline decltype(auto) cat(const metamap<T...>& a, const metamap<U...>& b) {
  return metamap<T..., U...>(*static_cast<const T*>(&a)..., *static_cast<const U*>(&b)...);
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_CAT

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_INTERSECTION
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_INTERSECTION

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_MAKE_METAMAP_SKIP
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_MAKE_METAMAP_SKIP

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_MAKE
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_MAKE

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SYMBOL_AST
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SYMBOL_AST


namespace li {

template <typename E> struct Exp {};

template <typename E> struct array_subscriptable;

template <typename E> struct callable;

template <typename E> struct assignable;

template <typename E> struct array_subscriptable;

template <typename M, typename... A>
struct function_call_exp : public array_subscriptable<function_call_exp<M, A...>>,
                           public callable<function_call_exp<M, A...>>,
                           public assignable<function_call_exp<M, A...>>,
                           public Exp<function_call_exp<M, A...>> {
  using assignable<function_call_exp<M, A...>>::operator=;

  function_call_exp(const M& m, A&&... a) : method(m), args(std::forward<A>(a)...) {}

  M method;
  std::tuple<A...> args;
};

template <typename O, typename M>
struct array_subscript_exp : public array_subscriptable<array_subscript_exp<O, M>>,
                             public callable<array_subscript_exp<O, M>>,
                             public assignable<array_subscript_exp<O, M>>,
                             public Exp<array_subscript_exp<O, M>> {
  using assignable<array_subscript_exp<O, M>>::operator=;

  array_subscript_exp(const O& o, const M& m) : object(o), member(m) {}

  O object;
  M member;
};

template <typename L, typename R> struct assign_exp : public Exp<assign_exp<L, R>> {
  typedef L left_t;
  typedef R right_t;

  // template <typename V>
  // assign_exp(L l, V&& r) : left(l), right(std::forward<V>(r)) {}
  // template <typename V>
  inline assign_exp(L l, R r) : left(l), right(r) {}
  // template <typename V>
  // inline assign_exp(L l, const V& r) : left(l), right(r) {}

  L left;
  R right;
};

template <typename E> struct array_subscriptable {
public:
  // Member accessor
  template <typename S> constexpr auto operator[](S&& s) const {
    return array_subscript_exp<E, S>(*static_cast<const E*>(this), std::forward<S>(s));
  }
};

template <typename E> struct callable {
public:
  // Direct call.
  template <typename... A> constexpr auto operator()(A&&... args) const {
    return function_call_exp<E, A...>(*static_cast<const E*>(this), std::forward<A>(args)...);
  }
};

template <typename E> struct assignable {
public:
  template <typename L> auto operator=(L&& l) const {
    return assign_exp<E, L>(static_cast<const E&>(*this), std::forward<L>(l));
  }

  template <typename L> auto operator=(L&& l) {
    return assign_exp<E, L>(static_cast<E&>(*this), std::forward<L>(l));
  }

  template <typename T> auto operator=(const std::initializer_list<T>& l) const {
    return assign_exp<E, std::vector<T>>(static_cast<const E&>(*this), std::vector<T>(l));
  }
};

#define iod_query_declare_binary_op(OP, NAME)                                                      \
  template <typename A, typename B>                                                                \
  struct NAME##_exp : public assignable<NAME##_exp<A, B>>, public Exp<NAME##_exp<A, B>> {          \
    using assignable<NAME##_exp<A, B>>::operator=;                                                 \
    NAME##_exp() {}                                                                                \
    NAME##_exp(A&& a, B&& b) : lhs(std::forward<A>(a)), rhs(std::forward<B>(b)) {}                 \
    typedef A lhs_type;                                                                            \
    typedef B rhs_type;                                                                            \
    lhs_type lhs;                                                                                  \
    rhs_type rhs;                                                                                  \
  };                                                                                               \
  template <typename A, typename B>                                                                \
  inline std::enable_if_t<std::is_base_of<Exp<A>, A>::value || std::is_base_of<Exp<B>, B>::value,  \
                          NAME##_exp<A, B>>                                                        \
  operator OP(const A& b, const B& a) {                                                            \
    return NAME##_exp<std::decay_t<A>, std::decay_t<B>>{b, a};                                     \
  }

iod_query_declare_binary_op(+, plus);
iod_query_declare_binary_op(-, minus);
iod_query_declare_binary_op(*, mult);
iod_query_declare_binary_op(/, div);
iod_query_declare_binary_op(<<, shiftl);
iod_query_declare_binary_op(>>, shiftr);
iod_query_declare_binary_op(<, inf);
iod_query_declare_binary_op(<=, inf_eq);
iod_query_declare_binary_op(>, sup);
iod_query_declare_binary_op(>=, sup_eq);
iod_query_declare_binary_op(==, eq);
iod_query_declare_binary_op(!=, neq);
iod_query_declare_binary_op(&, logical_and);
iod_query_declare_binary_op (^, logical_xor);
iod_query_declare_binary_op(|, logical_or);
iod_query_declare_binary_op(&&, and);
iod_query_declare_binary_op(||, or);

#undef iod_query_declare_binary_op

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SYMBOL_AST

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SYMBOL_SYMBOL
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SYMBOL_SYMBOL


namespace li {

template <typename S>
class symbol : public assignable<S>,
               public array_subscriptable<S>,
               public callable<S>,
               public Exp<S> {};
} // namespace li

#ifdef LI_SYMBOL
#undef LI_SYMBOL
#endif

#define LI_SYMBOL(NAME)                                                                            \
  namespace s {                                                                                    \
  struct NAME##_t : li::symbol<NAME##_t> {                                                         \
                                                                                                   \
    using assignable<NAME##_t>::operator=;                                                         \
                                                                                                   \
    inline constexpr bool operator==(NAME##_t) { return true; }                                    \
    template <typename T> inline constexpr bool operator==(T) { return false; }                    \
                                                                                                   \
    template <typename V> struct variable_t {                                                      \
      typedef NAME##_t _iod_symbol_type;                                                           \
      typedef V _iod_value_type;                                                                   \
      V NAME;                                                                                      \
    };                                                                                             \
                                                                                                   \
    template <typename T, typename... A>                                                           \
    static inline decltype(auto) symbol_method_call(T&& o, A... args) {                            \
      return o.NAME(args...);                                                                      \
    }                                                                                              \
    template <typename T, typename... A> static inline auto& symbol_member_access(T&& o) {         \
      return o.NAME;                                                                               \
    }                                                                                              \
    template <typename T>                                                                          \
    static constexpr auto has_getter(int)                                                          \
        -> decltype(std::declval<T>().NAME(), std::true_type{}) {                                  \
      return {};                                                                                   \
    }                                                                                              \
    template <typename T> static constexpr auto has_getter(long) { return std::false_type{}; }     \
    template <typename T>                                                                          \
    static constexpr auto has_member(int) -> decltype(std::declval<T>().NAME, std::true_type{}) {  \
      return {};                                                                                   \
    }                                                                                              \
    template <typename T> static constexpr auto has_member(long) { return std::false_type{}; }     \
                                                                                                   \
    static inline auto symbol_string() { return #NAME; }                                           \
  };                                                                                               \
  static constexpr NAME##_t NAME;                                                                  \
  }

namespace li {

template <typename S> inline decltype(auto) make_variable(S s, char const v[]) {
  typedef typename S::template variable_t<const char*> ret;
  return ret{v};
}

template <typename V, typename S> inline decltype(auto) make_variable(S s, V v) {
  typedef typename S::template variable_t<std::remove_const_t<std::remove_reference_t<V>>> ret;
  return ret{v};
}

template <typename K, typename V> inline decltype(auto) make_variable_reference(K s, V&& v) {
  typedef typename K::template variable_t<V> ret;
  return ret{v};
}

template <typename T, typename S, typename... A>
static inline decltype(auto) symbol_method_call(T&& o, S, A... args) {
  return S::symbol_method_call(o, std::forward<A>(args)...);
}

template <typename T, typename S> static inline decltype(auto) symbol_member_access(T&& o, S) {
  return S::symbol_member_access(o);
}

template <typename T, typename S> constexpr auto has_member(T&& o, S) {
  return S::template has_member<T>(0);
}
template <typename T, typename S> constexpr auto has_member() {
  return S::template has_member<T>(0);
}

template <typename T, typename S> constexpr auto has_getter(T&& o, S) {
  return decltype(S::template has_getter<T>(0)){};
}
template <typename T, typename S> constexpr auto has_getter() {
  return decltype(S::template has_getter<T>(0)){};
}

template <typename S, typename T> struct CANNOT_FIND_REQUESTED_MEMBER_IN_TYPE {};

template <typename T, typename S> decltype(auto) symbol_member_or_getter_access(T&& o, S) {
  if constexpr (has_getter<T, S>()) {
    return symbol_method_call(o, S{});
  } else if constexpr (has_member<T, S>()) {
    return symbol_member_access(o, S{});
  } else {
    return CANNOT_FIND_REQUESTED_MEMBER_IN_TYPE<S, T>::error;
  }
}

template <typename S> auto symbol_string(symbol<S> v) { return S::symbol_string(); }

template <typename V> auto symbol_string(V v, typename V::_iod_symbol_type* = 0) {
  return V::_iod_symbol_type::symbol_string();
}
} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SYMBOL_SYMBOL


namespace li {

template <typename... Ms> struct metamap;

namespace internal {

template <typename S, typename V> decltype(auto) exp_to_variable_ref(const assign_exp<S, V>& e) {
  return make_variable_reference(S{}, e.right);
}

template <typename S, typename V> decltype(auto) exp_to_variable(const assign_exp<S, V>& e) {
  typedef std::remove_const_t<std::remove_reference_t<V>> vtype;
  return make_variable(S{}, e.right);
}

template <typename S> decltype(auto) exp_to_variable(const symbol<S>& e) {
  return exp_to_variable(S() = int());
}

template <typename... T> inline decltype(auto) make_metamap_helper(T&&... args) {
  return metamap<T...>(std::forward<T>(args)...);
}

} // namespace internal

// Store copies of values in the map
static struct {
  template <typename... T> inline decltype(auto) operator()(T&&... args) const {
    // Copy values.
    return internal::make_metamap_helper(internal::exp_to_variable(std::forward<T>(args))...);
  }
} mmm;

// Store references of values in the map
template <typename... T> inline decltype(auto) make_metamap_reference(T&&... args) {
  // Keep references.
  return internal::make_metamap_helper(internal::exp_to_variable_ref(std::forward<T>(args))...);
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_MAKE


namespace li {

struct skip {};
static struct {

  template <typename... M, typename... T>
  inline decltype(auto) run(metamap<M...> map, skip, T&&... args) const {
    return run(map, std::forward<T>(args)...);
  }

  template <typename T1, typename... M, typename... T>
  inline decltype(auto) run(metamap<M...> map, T1&& a, T&&... args) const {
    return run(
        cat(map, internal::make_metamap_helper(internal::exp_to_variable(std::forward<T1>(a)))),
        std::forward<T>(args)...);
  }

  template <typename... M> inline decltype(auto) run(metamap<M...> map) const { return map; }

  template <typename... T> inline decltype(auto) operator()(T&&... args) const {
    // Copy values.
    return run(metamap<>{}, std::forward<T>(args)...);
  }

} make_metamap_skip;

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_MAKE_METAMAP_SKIP

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_MAP_REDUCE
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_MAP_REDUCE

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_TUPLE_UTILS
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_TUPLE_UTILS


namespace li {

template <typename... E, typename F> void apply_each(F&& f, E&&... e) {
  (void)std::initializer_list<int>{((void)f(std::forward<E>(e)), 0)...};
}

template <typename... E, typename F, typename R>
auto tuple_map_reduce_impl(F&& f, R&& reduce, E&&... e) {
  return reduce(f(std::forward<E>(e))...);
}

template <typename T, typename F> void tuple_map(T&& t, F&& f) {
  return std::apply([&](auto&&... e) { apply_each(f, std::forward<decltype(e)>(e)...); },
                    std::forward<T>(t));
}

template <typename T, typename F> auto tuple_reduce(T&& t, F&& f) {
  return std::apply(std::forward<F>(f), std::forward<T>(t));
}

template <typename T, typename F, typename R>
decltype(auto) tuple_map_reduce(T&& m, F map, R reduce) {
  auto fun = [&](auto... e) { return tuple_map_reduce_impl(map, reduce, e...); };
  return std::apply(fun, m);
}

template <typename F> inline std::tuple<> tuple_filter_impl() { return std::make_tuple(); }

template <typename F, typename... M, typename M1> auto tuple_filter_impl(M1 m1, M... m) {
  if constexpr (std::is_same<M1, F>::value)
    return tuple_filter_impl<F>(m...);
  else
    return std::tuple_cat(std::make_tuple(m1), tuple_filter_impl<F>(m...));
}

template <typename F, typename... M> auto tuple_filter(const std::tuple<M...>& m) {

  auto fun = [](auto... e) { return tuple_filter_impl<F>(e...); };
  return std::apply(fun, m);
}

} // namespace li
#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_TUPLE_UTILS


namespace li {

// Map a function(key, value) on all kv pair
template <typename... M, typename F> void map(const metamap<M...>& m, F fun) {
  auto apply = [&](auto key) -> decltype(auto) { return fun(key, m[key]); };

  apply_each(apply, typename M::_iod_symbol_type{}...);
}

// Map a function(key, value) on all kv pair. Ensure that the calling order
// is kept.
// template <typename O, typename F>
// void map_sequential2(F fun, O& obj)
// {}
// template <typename O, typename M1, typename... M, typename F>
// void map_sequential2(F fun, O& obj, M1 m1, M... ms)
// {
//   auto apply = [&] (auto key) -> decltype(auto)
//     {
//       return fun(key, obj[key]);
//     };

//   apply(m1);
//   map_sequential2(fun, obj, ms...);
// }
// template <typename... M, typename F>
// void map_sequential(const metamap<M...>& m, F fun)
// {
//   auto apply = [&] (auto key) -> decltype(auto)
//     {
//       return fun(key, m[key]);
//     };

//   map_sequential2(fun, m, typename M::_iod_symbol_type{}...);
// }

// Map a function(key, value) on all kv pair (non const).
template <typename... M, typename F> void map(metamap<M...>& m, F fun) {
  auto apply = [&](auto key) -> decltype(auto) { return fun(key, m[key]); };

  apply_each(apply, typename M::_iod_symbol_type{}...);
}

template <typename... E, typename F, typename R> auto apply_each2(F&& f, R&& r, E&&... e) {
  return r(f(std::forward<E>(e))...);
  //(void)std::initializer_list<int>{
  //  ((void)f(std::forward<E>(e)), 0)...};
}

// Map a function(key, value) on all kv pair an reduce
// all the results value with the reduce(r1, r2, ...) function.
template <typename... M, typename F, typename R>
decltype(auto) map_reduce(const metamap<M...>& m, F map, R reduce) {
  auto apply = [&](auto key) -> decltype(auto) {
    // return map(key, std::forward<decltype(m[key])>(m[key]));
    return map(key, m[key]);
  };

  return apply_each2(apply, reduce, typename M::_iod_symbol_type{}...);
  // return reduce(apply(typename M::_iod_symbol_type{})...);
}

// Map a function(key, value) on all kv pair an reduce
// all the results value with the reduce(r1, r2, ...) function.
template <typename... M, typename R> decltype(auto) reduce(const metamap<M...>& m, R reduce) {
  return reduce(m[typename M::_iod_symbol_type{}]...);
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_MAP_REDUCE



namespace li {

template <typename... T, typename... U>
inline decltype(auto) intersection(const metamap<T...>& a, const metamap<U...>& b) {
  return map_reduce(a,
                    [&](auto k, auto&& v) -> decltype(auto) {
                      if constexpr (has_key<metamap<U...>, decltype(k)>()) {
                        return k = std::forward<decltype(v)>(v);
                      } else
                        return skip{};
                    },
                    make_metamap_skip);
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_INTERSECTION

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_SUBSTRACT
#define LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_SUBSTRACT


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

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_ALGORITHMS_SUBSTRACT


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_METAMAP_METAMAP


#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_SYMBOLS
#define LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_SYMBOLS

#ifndef LI_SYMBOL_append
#define LI_SYMBOL_append
    LI_SYMBOL(append)
#endif

#ifndef LI_SYMBOL_json_key
#define LI_SYMBOL_json_key
    LI_SYMBOL(json_key)
#endif

#ifndef LI_SYMBOL_name
#define LI_SYMBOL_name
    LI_SYMBOL(name)
#endif

#ifndef LI_SYMBOL_type
#define LI_SYMBOL_type
    LI_SYMBOL(type)
#endif


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_SYMBOLS


namespace li {

template <typename O> inline decltype(auto) wrap_json_output_stream(O&& s) {
  return mmm(s::append = [&s](char c) { s << c; });
}

inline decltype(auto) wrap_json_output_stream(std::ostringstream& s) {
  return mmm(s::append = [&s](char c) { s << c; });
}

inline decltype(auto) wrap_json_output_stream(std::string& s) {
  return mmm(s::append = [&s](char c) { s.append(1, c); });
}

inline decltype(auto) wrap_json_input_stream(std::istringstream& s) { return s; }
inline decltype(auto) wrap_json_input_stream(std::stringstream& s) { return s; }
inline decltype(auto) wrap_json_input_stream(decode_stringstream& s) { return s; }
inline decltype(auto) wrap_json_input_stream(const std::string& s) { return std::istringstream(s); }
inline decltype(auto) wrap_json_input_stream(const char* s) {
  return std::istringstream(std::string(s));
}
inline decltype(auto) wrap_json_input_stream(const std::string_view& s) {
  return std::istringstream(std::string(s));
}

namespace unicode_impl {
template <typename S, typename T> auto json_to_utf8(S&& s, T&& o);

template <typename S, typename T> auto utf8_to_json(S&& s, T&& o);
} // namespace unicode_impl

template <typename I, typename O> auto json_to_utf8(I&& i, O&& o) {
  return unicode_impl::json_to_utf8(wrap_json_input_stream(std::forward<I>(i)),
                                    wrap_json_output_stream(std::forward<O>(o)));
}

template <typename I, typename O> auto utf8_to_json(I&& i, O&& o) {
  return unicode_impl::utf8_to_json(wrap_json_input_stream(std::forward<I>(i)),
                                    wrap_json_output_stream(std::forward<O>(o)));
}

enum json_encodings { UTF32BE, UTF32LE, UTF16BE, UTF16LE, UTF8 };

// Detection of encoding depending on the pattern of the
// first fourth characters.
auto detect_encoding(char a, char b, char c, char d) {
  // 00 00 00 xx  UTF-32BE
  // xx 00 00 00  UTF-32LE
  // 00 xx 00 xx  UTF-16BE
  // xx 00 xx 00  UTF-16LE
  // xx xx xx xx  UTF-8

  if (a == 0 and b == 0)
    return UTF32BE;
  else if (c == 0 and d == 0)
    return UTF32LE;
  else if (a == 0)
    return UTF16BE;
  else if (b == 0)
    return UTF16LE;
  else
    return UTF8;
}

// The JSON RFC escapes character codepoints prefixed with a \uXXXX (7-11 bits codepoints)
// or \uXXXX\uXXXX (20 bits codepoints)

// uft8 string have 4 kinds of character representation encoding the codepoint of the character.

// 1 byte : 0xxxxxxx  -> 7 bits codepoint ASCII chars from 0x00 to 0x7F
// 2 bytes: 110xxxxx 10xxxxxx -> 11 bits codepoint
// 3 bytes: 1110xxxx 10xxxxxx 10xxxxxx -> 11 bits codepoint

// 1 and 3 bytes representation are escaped as \uXXXX with X a char in the 0-9A-F range. It
// is possible since the codepoint is less than 16 bits.

// the 4 bytes representation uses the UTF-16 surrogate pair (high and low surrogate).

// The high surrogate is in the 0xD800..0xDBFF range (HR) and
// the low surrogate is in the 0xDC00..0xDFFF range (LR).

// to encode a given 20bits codepoint c to the surrogate pair.
//  - substract 0x10000 to c
//  - separate the result in a high (first 10 bits) and low (last 10bits) surrogate.
//  - Add 0xD800 to the high surrogate
//  - Add 0xDC00 to the low surrogate
//  - the 32 bits code is (high << 16) + low.

// and to json-escape the high-low(H-L) surrogates representation (16+16bits):
//  - Check that H and L are respectively in the HR and LR ranges.
//  - add to H-L 0x0001_0000 - 0xD800_DC00 to get the 20bits codepoint c.
//  - Encode the codepoint in a string of \uXXXX\uYYYY with X and Y the respective hex digits
//    of the high and low sequence of 10 bits.

// In addition to utf8, JSON escape characters ( " \ / ) with a backslash and translate
// \n \r \t \b \r in their matching two characters string, for example '\n' to  '\' followed by 'n'.

namespace unicode_impl {
template <typename S, typename T> auto json_to_utf8(S&& s, T&& o) {
  // Convert a JSON string into an UTF-8 string.
  if (s.get() != '"')
    return JSON_KO; // make_json_error("json_to_utf8: JSON strings should start with a double
                    // quote.");

  while (true) {
    // Copy until we find the escaping backslash or the end of the string (double quote).
    while (s.peek() != EOF and s.peek() != '"' and s.peek() != '\\')
      o.append(s.get());

    // If eof found before the end of the string, return an error.
    if (s.eof())
      return JSON_KO; // make_json_error("json_to_utf8: Unexpected end of string when parsing a
                      // string.");

    // If end of json string, return
    if (s.peek() == '"') {
      break;
      return JSON_OK;
    }

    // Get the '\'.
    assert(s.peek() == '\\');
    s.get();

    switch (s.get()) {
      // List of escaped char from http://www.json.org/
    default:
      return JSON_KO; // make_json_error("json_to_utf8: Bad JSON escaped character.");
    case '"':
      o.append('"');
      break;
    case '\\':
      o.append('\\');
      break;
    case '/':
      o.append('/');
      break;
    case 'n':
      o.append('\n');
      break;
    case 'r':
      o.append('\r');
      break;
    case 't':
      o.append('\t');
      break;
    case 'b':
      o.append('\b');
      break;
    case 'f':
      o.append('\f');
      break;
    case 'u':
      char a, b, c, d;

      a = s.get();
      b = s.get();
      c = s.get();
      d = s.get();

      if (s.eof())
        return JSON_KO; // make_json_error("json_to_utf8: Unexpected end of string when decoding an
                        // utf8 character");

      auto decode_hex_c = [](char c) {
        if (c >= '0' and c <= '9')
          return c - '0';
        else
          return (10 + std::toupper(c) - 'A');
      };

      uint16_t x = (decode_hex_c(a) << 12) + (decode_hex_c(b) << 8) + (decode_hex_c(c) << 4) +
                   decode_hex_c(d);

      // If x in the  0xD800..0xDBFF range -> decode a surrogate pair \uXXXX\uYYYY -> 20 bits
      // codepoint.
      if (x >= 0xD800 and x <= 0xDBFF) {
        if (s.get() != '\\' or s.get() != 'u')
          return JSON_KO; // make_json_error("json_to_utf8: Missing low surrogate.");

        uint16_t y = (decode_hex_c(s.get()) << 12) + (decode_hex_c(s.get()) << 8) +
                     (decode_hex_c(s.get()) << 4) + decode_hex_c(s.get());

        if (s.eof())
          return JSON_KO; // make_json_error("json_to_utf8: Unexpected end of string when decoding
                          // an utf8 character");

        x -= 0xD800;
        y -= 0xDC00;

        int cp = (x << 10) + y + 0x10000;

        o.append(0b11110000 | (cp >> 18));
        o.append(0b10000000 | ((cp & 0x3F000) >> 12));
        o.append(0b10000000 | ((cp & 0x00FC0) >> 6));
        o.append(0b10000000 | (cp & 0x003F));

      }
      // else encode the codepoint with the 1-2, or 3 bytes utf8 representation.
      else {
        if (x <= 0x007F) // 7bits codepoints, ASCII 0xxxxxxx.
        {
          o.append(uint8_t(x));
        } else if (x >= 0x0080 and x <= 0x07FF) // 11bits codepoint -> 110xxxxx 10xxxxxx
        {
          o.append(0b11000000 | (x >> 6));
          o.append(0b10000000 | (x & 0x003F));
        } else if (x >= 0x0800 and x <= 0xFFFF) // 16bits codepoint -> 1110xxxx 10xxxxxx 10xxxxxx
        {
          o.append(0b11100000 | (x >> 12));
          o.append(0b10000000 | ((x & 0x0FC0) >> 6));
          o.append(0b10000000 | (x & 0x003F));
        } else
          return JSON_KO; // make_json_error("json_to_utf8: Bad UTF8 codepoint.");
      }
      break;
    }
  }

  if (s.get() != '"')
    return JSON_KO; // make_json_error("JSON strings must end with a double quote.");

  return JSON_OK; // json_no_error();
}

template <typename S, typename T> auto utf8_to_json(S&& s, T&& o) {
  o.append('"');

  auto encode_16bits = [&](uint16_t b) {
    const char lt[] = "0123456789ABCDEF";
    o.append(lt[b >> 12]);
    o.append(lt[(b & 0x0F00) >> 8]);
    o.append(lt[(b & 0x00F0) >> 4]);
    o.append(lt[b & 0x000F]);
  };

  while (!s.eof()) {
    // 7-bits codepoint
    while (s.good() and s.peek() <= 0x7F and s.peek() != EOF) {
      switch (s.peek()) {
      case '"':
        o.append('\\');
        o.append('"');
        break;
      case '\\':
        o.append('\\');
        o.append('\\');
        break;
        // case '/': o.append('/'); break; Do not escape /
      case '\n':
        o.append('\\');
        o.append('n');
        break;
      case '\r':
        o.append('\\');
        o.append('r');
        break;
      case '\t':
        o.append('\\');
        o.append('t');
        break;
      case '\b':
        o.append('\\');
        o.append('b');
        break;
      case '\f':
        o.append('\\');
        o.append('f');
        break;
      default:
        o.append(s.peek());
      }
      s.get();
    }

    if (s.eof())
      break;

    // uft8 prefix \u.
    o.append('\\');
    o.append('u');

    uint8_t c1 = s.get();
    uint8_t c2 = s.get();
    {
      // extract codepoints.
      if (c1 < 0b11100000) // 11bits - 2 char: 110xxxxx	10xxxxxx
      {
        uint16_t cp = ((c1 & 0b00011111) << 6) + (c2 & 0b00111111);
        if (cp >= 0x0080 and cp <= 0x07FF)
          encode_16bits(cp);
        else
          return JSON_KO;         // make_json_error("utf8_to_json: Bad UTF8 codepoint.");
      } else if (c1 < 0b11110000) // 16 bits - 3 char: 1110xxxx	10xxxxxx	10xxxxxx
      {
        uint16_t cp = ((c1 & 0b00001111) << 12) + ((c2 & 0b00111111) << 6) + (s.get() & 0b00111111);

        if (cp >= 0x0800 and cp <= 0xFFFF)
          encode_16bits(cp);
        else
          return JSON_KO; // make_json_error("utf8_to_json: Bad UTF8 codepoint.");
      } else              // 21 bits - 4 chars: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
      {
        int cp = ((c1 & 0b00000111) << 18) + ((c2 & 0b00111111) << 12) +
                 ((s.get() & 0b00111111) << 6) + (s.get() & 0b00111111);

        cp -= 0x10000;

        uint16_t H = (cp >> 10) + 0xD800;
        uint16_t L = (cp & 0x03FF) + 0xDC00;

        // check if we are in the right range.
        // The high surrogate is in the 0xD800..0xDBFF range (HR) and
        // the low surrogate is in the 0xDC00..0xDFFF range (LR).
        assert(H >= 0xD800 and H <= 0xDBFF and L >= 0xDC00 and L <= 0xDFFF);

        encode_16bits(H);
        o.append('\\');
        o.append('u');
        encode_16bits(L);
      }
    }
  }
  o.append('"');
  return JSON_OK;
}
} // namespace unicode_impl
} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_UNICODE

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_UTILS
#define LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_UTILS



namespace li {

template <typename T> struct json_object_base;

template <typename T> struct json_object_;
template <typename T> struct json_vector_;
template <typename V> struct json_value_;
template <typename... T> struct json_tuple_;
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

template <typename V> auto to_json_schema(V v) { return json_value_<V>{}; }

template <typename... M> auto to_json_schema(const metamap<M...>& m);

template <typename V> auto to_json_schema(const std::vector<V>& arr) {
  auto elt = to_json_schema(decltype(arr[0]){});
  return json_vector_<decltype(elt)>{elt};
}

template <typename... V> auto to_json_schema(const std::tuple<V...>& arr) {
  return json_tuple_<decltype(to_json_schema(V{}))...>(to_json_schema(V{})...);
}

template <typename... M> auto to_json_schema(const metamap<M...>& m) {
  auto tuple_maker = [](auto&&... t) { return std::make_tuple(std::forward<decltype(t)>(t)...); };

  auto entities = map_reduce(
      m, [](auto k, auto v) { return mmm(s::name = k, s::type = to_json_schema(v)); }, tuple_maker);

  return json_object_<decltype(entities)>(entities);
}

template <typename... E> auto json_object_to_metamap(const json_object_<std::tuple<E...>>& s) {
  auto make_kvs = [](auto... elt) { return std::make_tuple((elt.name = elt.type)...); };

  auto kvs = std::apply(make_kvs, s.entities);
  return std::apply(mmm, kvs);
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

template <typename E> constexpr auto json_is_vector(json_vector_<E>) -> std::true_type {
  return {};
}
template <typename E> constexpr auto json_is_vector(E) -> std::false_type { return {}; }

template <typename... E> constexpr auto json_is_tuple(json_tuple_<E...>) -> std::true_type {
  return {};
}
template <typename E> constexpr auto json_is_tuple(E) -> std::false_type { return {}; }

template <typename E> constexpr auto json_is_object(json_object_<E>) -> std::true_type {
  return {};
}
template <typename E> constexpr auto json_is_object(E) -> std::false_type { return {}; }

template <typename E> constexpr auto json_is_value(json_object_<E>) -> std::false_type {
  return {};
}
template <typename E> constexpr auto json_is_value(json_vector_<E>) -> std::false_type {
  return {};
}
template <typename... E> constexpr auto json_is_value(json_tuple_<E...>) -> std::false_type {
  return {};
}
template <typename E> constexpr auto json_is_value(E) -> std::true_type { return {}; }

template <typename T> constexpr auto is_std_optional(std::optional<T>) -> std::true_type;
template <typename T> constexpr auto is_std_optional(T) -> std::false_type;

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_UTILS


namespace li {

namespace impl {

template <typename S> struct json_parser {
  inline json_parser(S&& s) : ss(s) {}
  inline json_parser(S& s) : ss(s) {}

  inline decltype(auto) peek() { return ss.peek(); }
  inline decltype(auto) get() { return ss.get(); }

  inline void skip_one() { ss.get(); }

  inline bool eof() { return ss.eof(); }
  inline json_error_code eat(char c, bool skip_spaces = true) {
    if (skip_spaces)
      eat_spaces();

    char g = ss.get();
    if (g != c)
      return make_json_error("Unexpected char. Got '", char(g), "' expected ", c);
    return JSON_OK;
  }

  inline json_error_code eat(const char* str, bool skip_spaces = true) {
    if (skip_spaces)
      eat_spaces();

    const char* str_it = str;
    while (*str_it) {
      char g = ss.get();
      if (g != *str_it)
        return make_json_error("Unexpected char. Got '", char(g), "' expected '", *str_it,
                               "' when parsing string ", str);
      str_it++;
    }
    return JSON_OK;
  }

  template <typename... T> inline json_error_code make_json_error(T&&... t) {
    if (!error_stream)
      error_stream = new std::ostringstream();
    *error_stream << "json error: ";
    auto add = [this](auto w) { *error_stream << w; };
    apply_each(add, t...);
    return JSON_KO;
  }
  inline void eat_spaces() {
    while (ss.peek() >= 0 and ss.peek() < 33)
      ss.get();
  }

  template <typename X> struct JSON_INVALID_TYPE;

  // Integers and floating points.
  template <typename T> json_error_code fill(T& t) {

    if constexpr (std::is_floating_point<T>::value or std::is_integral<T>::value or
                  std::is_same<T, std::string_view>::value) {
      ss >> t;
      if (ss.bad())
        return make_json_error("Ill-formated value.");
      return JSON_OK;
    } else
      // The JSON decoder only parses floating-point, integral and string types.
      return JSON_INVALID_TYPE<T>::error;
  }

  // Strings
  inline json_error_code fill(std::string& str) {
    eat_spaces();
    str.clear();
    return json_to_utf8(ss, str);
  }

  template <typename T> inline json_error_code fill(std::optional<T>& opt) {
    opt.emplace();
    return fill(opt.value());
  }

  template <typename... T> inline json_error_code fill(std::variant<T...>& v) {
    if (auto err = eat('{'))
      return err;
    if (auto err = eat("\"idx\""))
      return err;
    if (auto err = eat(':'))
      return err;

    int idx = 0;
    fill(idx);
    if (auto err = eat(','))
      return err;
    if (auto err = eat("\"value\""))
      return err;
    if (auto err = eat(':'))
      return err;

    int cpt = 0;
    apply_each(
        [&](auto* x) {
          if (cpt == idx) {
            std::remove_pointer_t<decltype(x)> value{};
            fill(value);
            v = std::move(value);
          }
          cpt++;
        },
        (T*)nullptr...);

    if (auto err = eat('}'))
      return err;
    return JSON_OK;
  }

  S& ss;
  std::ostringstream* error_stream = nullptr;
};

template <typename P, typename O, typename S> json_error_code json_decode2(P& p, O& obj, S) {
  auto err = p.fill(obj);
  if (err)
    return err;
  else
    return JSON_OK;
}

template <typename P, typename O, typename S>
json_error_code json_decode2(P& p, O& obj, json_vector_<S> schema) {
  obj.clear();
  bool first = true;
  auto err = p.eat('[');
  if (err)
    return err;

  p.eat_spaces();
  while (p.peek() != ']') {
    if (!first) {
      if ((err = p.eat(',')))
        return err;
    }
    first = false;

    obj.resize(obj.size() + 1);
    if ((err = json_decode2(p, obj.back(), S{})))
      return err;
    p.eat_spaces();
  }

  if ((err = p.eat(']')))
    return err;
  else
    return JSON_OK;
}

template <typename F, typename... E, typename... T, std::size_t... I>
inline void json_decode_tuple_elements(F& decode_fun, std::tuple<T...>& tu,
                                       const std::tuple<E...>& schema, std::index_sequence<I...>) {
  (void)std::initializer_list<int>{((void)decode_fun(std::get<I>(tu), std::get<I>(schema)), 0)...};
}

template <typename P, typename... O, typename... S>
json_error_code json_decode2(P& p, std::tuple<O...>& tu, json_tuple_<S...> schema) {
  bool first = true;
  auto err = p.eat('[');
  if (err)
    return err;

  auto decode_one_element = [&first, &p, &err](auto& value, auto value_schema) {
    if (!first) {
      if ((err = p.eat(',')))
        return err;
    }
    first = false;
    if ((err = json_decode2(p, value, value_schema)))
      return err;
    p.eat_spaces();
    return JSON_OK;
  };

  json_decode_tuple_elements(decode_one_element, tu, schema.elements,
                             std::make_index_sequence<sizeof...(O)>{});

  if ((err = p.eat(']')))
    return err;
  else
    return JSON_OK;
}

template <typename P, typename O, typename S>
json_error_code json_decode2(P& p, O& obj, json_object_<S> schema) {
  json_error_code err;
  if ((err = p.eat('{')))
    return err;

  struct attr_info {
    bool filled;
    bool required;
    const char* name;
    int name_len;
    std::function<json_error_code(P&)> parse_value;
  };
  constexpr int n_members = std::tuple_size<decltype(schema.schema)>();
  attr_info A[n_members];
  int i = 0;
  auto prepare = [&](auto m) {
    A[i].filled = false;
    A[i].required = true;
    A[i].name = symbol_string(m.name);
    A[i].name_len = strlen(symbol_string(m.name));

    if constexpr (has_key(m, s::json_key)) {
      A[i].name = m.json_key;
    }

    if constexpr (decltype(is_std_optional(symbol_member_or_getter_access(obj, m.name))){}) {
      A[i].required = false;
    }

    A[i].parse_value = [m, &obj](P& p) {
      using V = decltype(symbol_member_or_getter_access(obj, m.name));
      using VS = decltype(get_or(m, s::type, json_value_<V>{}));

      if constexpr (decltype(json_is_value(VS{})){}) {
        if (auto err = p.fill(symbol_member_or_getter_access(obj, m.name)))
          return err;
        else
          return JSON_OK;
      } else {
        if (auto err = json_decode2(p, symbol_member_or_getter_access(obj, m.name), m.type))
          return err;
        else
          return JSON_OK;
      }
    };

    i++;
  };

  std::apply([&](auto... m) { apply_each(prepare, m...); }, schema.schema);

  while (p.peek() != '}') {

    bool found = false;
    if ((err = p.eat('"')))
      return err;
    char symbol[50];
    int symbol_size = 0;
    while (!p.eof() and p.peek() != '"' and symbol_size < 50)
      symbol[symbol_size++] = p.get();
    symbol[symbol_size] = 0;
    if ((err = p.eat('"', false)))
      return err;

    for (int i = 0; i < n_members; i++) {
      int len = A[i].name_len;
      if (!strncmp(symbol, A[i].name, len)) {
        if ((err = p.eat(':')))
          return err;
        if (A[i].filled)
          return p.make_json_error("Duplicate json key: ", A[i].name);

        if ((err = A[i].parse_value(p)))
          return err;
        A[i].filled = true;
        found = true;
        break;
      }
    }

    if (!found)
      return p.make_json_error("Unknown json key");
    p.eat_spaces();
    if (p.peek() == ',') {
      if ((err = p.eat(',')))
        return err;
    }
  }
  if ((err = p.eat('}')))
    return err;

  for (int i = 0; i < n_members; i++) {
    if (A[i].required and !A[i].filled)
      return p.make_json_error("Missing json key ", A[i].name);
  }
  return JSON_OK;
}

template <typename C, typename O, typename S> json_error json_decode(C& input, O& obj, S schema) {
  auto stream = decode_stringstream(input);
  json_parser<decode_stringstream> p(stream);
  if (json_decode2(p, obj, schema))
    return json_error{1, p.error_stream ? p.error_stream->str() : "Json error"};
  else
    return json_error{0};
}

} // namespace impl

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_DECODER

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_ENCODER
#define LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_ENCODER


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

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_ENCODER


namespace li {

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
  json_vector_() = default;
  json_vector_(const T& s) : schema(s) {}
  T schema;
};

template <typename... S> auto json_vector(S&&... s) {
  auto obj = json_object(std::forward<S>(s)...);
  return json_vector_<decltype(obj)>{obj};
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

template <typename C, typename M> decltype(auto) json_decode(C& input, M& obj) {
  return impl::to_json_schema(obj).decode(input, obj);
}

template <typename C, typename M> decltype(auto) json_encode(C& output, const M& obj) {
  impl::to_json_schema(obj).encode(output, obj);
}

template <typename M> auto json_encode(const M& obj) {
  return std::move(impl::to_json_schema(obj).encode(obj));
}

template <typename A, typename B, typename... C>
auto json_encode(const assign_exp<A, B>& exp, C... c) {
  auto obj = mmm(exp, c...);
  return impl::to_json_schema(obj).encode(obj);
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_JSON_JSON

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_COMMON
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_COMMON


namespace li {
struct sql_blob : public std::string {
  using std::string::string;
  using std::string::operator=;

  sql_blob() : std::string() {}
};

struct sql_null_t {};
static sql_null_t null;

template <unsigned SIZE> struct sql_varchar : public std::string {
  using std::string::string;
  using std::string::operator=;

  sql_varchar() : std::string() {}
};
} // namespace li
#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_COMMON

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_ORM
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_ORM

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_SYMBOLS
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_SYMBOLS

#ifndef LI_SYMBOL_blocking
#define LI_SYMBOL_blocking
    LI_SYMBOL(blocking)
#endif

#ifndef LI_SYMBOL_create_secret_key
#define LI_SYMBOL_create_secret_key
    LI_SYMBOL(create_secret_key)
#endif

#ifndef LI_SYMBOL_date_thread
#define LI_SYMBOL_date_thread
    LI_SYMBOL(date_thread)
#endif

#ifndef LI_SYMBOL_hash_password
#define LI_SYMBOL_hash_password
    LI_SYMBOL(hash_password)
#endif

#ifndef LI_SYMBOL_https_cert
#define LI_SYMBOL_https_cert
    LI_SYMBOL(https_cert)
#endif

#ifndef LI_SYMBOL_https_key
#define LI_SYMBOL_https_key
    LI_SYMBOL(https_key)
#endif

#ifndef LI_SYMBOL_id
#define LI_SYMBOL_id
    LI_SYMBOL(id)
#endif

#ifndef LI_SYMBOL_linux_epoll
#define LI_SYMBOL_linux_epoll
    LI_SYMBOL(linux_epoll)
#endif

#ifndef LI_SYMBOL_name
#define LI_SYMBOL_name
    LI_SYMBOL(name)
#endif

#ifndef LI_SYMBOL_non_blocking
#define LI_SYMBOL_non_blocking
    LI_SYMBOL(non_blocking)
#endif

#ifndef LI_SYMBOL_nthreads
#define LI_SYMBOL_nthreads
    LI_SYMBOL(nthreads)
#endif

#ifndef LI_SYMBOL_one_thread_per_connection
#define LI_SYMBOL_one_thread_per_connection
    LI_SYMBOL(one_thread_per_connection)
#endif

#ifndef LI_SYMBOL_path
#define LI_SYMBOL_path
    LI_SYMBOL(path)
#endif

#ifndef LI_SYMBOL_primary_key
#define LI_SYMBOL_primary_key
    LI_SYMBOL(primary_key)
#endif

#ifndef LI_SYMBOL_read_only
#define LI_SYMBOL_read_only
    LI_SYMBOL(read_only)
#endif

#ifndef LI_SYMBOL_select
#define LI_SYMBOL_select
    LI_SYMBOL(select)
#endif

#ifndef LI_SYMBOL_server_thread
#define LI_SYMBOL_server_thread
    LI_SYMBOL(server_thread)
#endif

#ifndef LI_SYMBOL_session_id
#define LI_SYMBOL_session_id
    LI_SYMBOL(session_id)
#endif

#ifndef LI_SYMBOL_update_secret_key
#define LI_SYMBOL_update_secret_key
    LI_SYMBOL(update_secret_key)
#endif

#ifndef LI_SYMBOL_user_id
#define LI_SYMBOL_user_id
    LI_SYMBOL(user_id)
#endif


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_SYMBOLS


#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SYMBOLS
#define LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SYMBOLS

#ifndef LI_SYMBOL_ATTR
#define LI_SYMBOL_ATTR
    LI_SYMBOL(ATTR)
#endif

#ifndef LI_SYMBOL_after_insert
#define LI_SYMBOL_after_insert
    LI_SYMBOL(after_insert)
#endif

#ifndef LI_SYMBOL_after_remove
#define LI_SYMBOL_after_remove
    LI_SYMBOL(after_remove)
#endif

#ifndef LI_SYMBOL_after_update
#define LI_SYMBOL_after_update
    LI_SYMBOL(after_update)
#endif

#ifndef LI_SYMBOL_auto_increment
#define LI_SYMBOL_auto_increment
    LI_SYMBOL(auto_increment)
#endif

#ifndef LI_SYMBOL_before_insert
#define LI_SYMBOL_before_insert
    LI_SYMBOL(before_insert)
#endif

#ifndef LI_SYMBOL_before_remove
#define LI_SYMBOL_before_remove
    LI_SYMBOL(before_remove)
#endif

#ifndef LI_SYMBOL_before_update
#define LI_SYMBOL_before_update
    LI_SYMBOL(before_update)
#endif

#ifndef LI_SYMBOL_charset
#define LI_SYMBOL_charset
    LI_SYMBOL(charset)
#endif

#ifndef LI_SYMBOL_computed
#define LI_SYMBOL_computed
    LI_SYMBOL(computed)
#endif

#ifndef LI_SYMBOL_database
#define LI_SYMBOL_database
    LI_SYMBOL(database)
#endif

#ifndef LI_SYMBOL_host
#define LI_SYMBOL_host
    LI_SYMBOL(host)
#endif

#ifndef LI_SYMBOL_password
#define LI_SYMBOL_password
    LI_SYMBOL(password)
#endif

#ifndef LI_SYMBOL_port
#define LI_SYMBOL_port
    LI_SYMBOL(port)
#endif

#ifndef LI_SYMBOL_primary_key
#define LI_SYMBOL_primary_key
    LI_SYMBOL(primary_key)
#endif

#ifndef LI_SYMBOL_read_access
#define LI_SYMBOL_read_access
    LI_SYMBOL(read_access)
#endif

#ifndef LI_SYMBOL_read_only
#define LI_SYMBOL_read_only
    LI_SYMBOL(read_only)
#endif

#ifndef LI_SYMBOL_synchronous
#define LI_SYMBOL_synchronous
    LI_SYMBOL(synchronous)
#endif

#ifndef LI_SYMBOL_user
#define LI_SYMBOL_user
    LI_SYMBOL(user)
#endif

#ifndef LI_SYMBOL_validate
#define LI_SYMBOL_validate
    LI_SYMBOL(validate)
#endif

#ifndef LI_SYMBOL_write_access
#define LI_SYMBOL_write_access
    LI_SYMBOL(write_access)
#endif


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SYMBOLS


namespace li {

struct sqlite_tag;
struct mysql_tag;

using s::auto_increment;
using s::primary_key;
using s::read_only;

template <typename SCHEMA, typename C> struct sql_orm {

  typedef decltype(std::declval<SCHEMA>().all_fields()) O;
  typedef O object_type;

  sql_orm(SCHEMA& schema, C con) : schema_(schema), con_(con) {}

  template <typename S, typename... A> void call_callback(S s, A&&... args) {
    if constexpr (has_key<decltype(schema_.get_callbacks())>(S{}))
      return schema_.get_callbacks()[s](args...);
  }

  inline auto drop_table_if_exists() {
    con_(std::string("DROP TABLE IF EXISTS ") + schema_.table_name());
    return *this;
  }

  inline auto create_table_if_not_exists() {
    std::ostringstream ss;
    ss << "CREATE TABLE if not exists " << schema_.table_name() << " (";

    bool first = true;
    li::tuple_map(schema_.all_info(), [&](auto f) {
      auto f2 = schema_.get_field(f);
      typedef decltype(f) F;
      typedef decltype(f2) F2;
      typedef typename F2::left_t K;
      typedef typename F2::right_t V;

      bool auto_increment = SCHEMA::template is_auto_increment<F>::value;
      bool primary_key = SCHEMA::template is_primary_key<F>::value;
      K k{};
      V v{};

      if (!first)
        ss << ", ";
      ss << li::symbol_string(k) << " " << con_.type_to_string(v);

      if (std::is_same<typename C::db_tag, sqlite_tag>::value) {
        if (auto_increment || primary_key)
          ss << " PRIMARY KEY ";
      }

      if (std::is_same<typename C::db_tag, mysql_tag>::value) {
        if (auto_increment)
          ss << " AUTO_INCREMENT NOT NULL";
        if (primary_key)
          ss << " PRIMARY KEY ";
      }

      // To activate when pgsql_connection is implemented.
      // if (std::is_same<C, pgsql_connection>::value and
      //     m.attributes().has(s::auto_increment))
      //   ss << " SERIAL ";

      first = false;
    });
    ss << ");";
    try {
      con_(ss.str())();
    } catch (std::exception e) {
      std::cerr << "Warning: Silicon could not create the " << schema_.table_name() << " sql table."
                << std::endl
                << "You can ignore this message if the table already exists."
                << "The sql error is: " << e.what() << std::endl;
    }
    return *this;
  }

  template <typename W> void where_clause(W&& cond, std::ostringstream& ss) {
    ss << " WHERE ";
    bool first = true;
    map(cond, [&](auto k, auto v) {
      if (!first)
        ss << " and ";
      first = false;
      ss << li::symbol_string(k) << " = ? ";
    });
    ss << " ";
  }

  template <typename... W, typename... A> auto find_one(metamap<W...> where, A&&... cb_args) {
    std::ostringstream ss;
    O o;
    ss << "SELECT ";
    bool first = true;
    li::map(o, [&](auto k, auto v) {
      if (!first)
        ss << ",";
      first = false;
      ss << li::symbol_string(k);
    });

    ss << " FROM " << schema_.table_name();
    where_clause(where, ss);
    ss << "LIMIT 1";
    auto stmt = con_.prepare(ss.str());

    auto res = li::tuple_reduce(metamap_values(where), stmt).template read_optional<O>();
    if (res)
      call_callback(s::read_access, o, cb_args...);
    return res;
  }

  template <typename A, typename B, typename... O, typename... W>
  auto find_one(metamap<O...>&& o, assign_exp<A, B> w1, W... ws) {
    return find_one(cat(o, mmm(w1)), ws...);
  }
  template <typename A, typename B, typename... W> auto find_one(assign_exp<A, B> w1, W... ws) {
    return find_one(mmm(w1), ws...);
  }

  template <typename W> bool exists(W&& cond) {
    std::ostringstream ss;
    O o;
    ss << "SELECT count(*) FROM " << schema_.table_name();
    where_clause(cond, ss);
    ss << "LIMIT 1";

    auto stmt = con_.prepare(ss.str());

    return li::tuple_reduce(metamap_values(cond), stmt).template read<int>();
  }

  template <typename A, typename B, typename... W> auto exists(assign_exp<A, B> w1, W... ws) {
    return exists(mmm(w1, ws...));
  }
  // Save a ll fields except auto increment.
  // The db will automatically fill auto increment keys.
  template <typename N, typename... A> long long int insert(N&& o, A&&... cb_args) {
    std::ostringstream ss;
    std::ostringstream vs;

    auto values = schema_.without_auto_increment();
    map(o, [&](auto k, auto& v) { values[k] = o[k]; });
    // auto values = intersection(o, schema_.without_auto_increment());

    call_callback(s::validate, values, cb_args...);
    call_callback(s::before_insert, values, cb_args...);
    ss << "INSERT into " << schema_.table_name() << "(";

    bool first = true;
    li::map(values, [&](auto k, auto v) {
      if (!first) {
        ss << ",";
        vs << ",";
      }
      first = false;
      ss << li::symbol_string(k);
      vs << "?";
    });

    ss << ") VALUES (" << vs.str() << ")";
    auto req = con_.prepare(ss.str());
    li::reduce(values, req);

    call_callback(s::after_insert, o, cb_args...);

    return req.last_insert_id();
  };
  template <typename A, typename B, typename... O, typename... W>
  long long int insert(metamap<O...>&& o, assign_exp<A, B> w1, W... ws) {
    return insert(cat(o, mmm(w1)), ws...);
  }
  template <typename A, typename B, typename... W>
  long long int insert(assign_exp<A, B> w1, W... ws) {
    return insert(mmm(w1), ws...);
  }

  // template <typename S, typename V, typename... A>
  // long long int insert(const assign_exp<S, V>& a, A&&... tail) {
  //   auto m = mmm(a, tail...);
  //   return insert(m);
  // }

  // Iterate on all the rows of the table.
  template <typename F> void forall(F f) {
    std::ostringstream ss;
    ss << "SELECT * from " << schema_.table_name();
    con_(ss.str()).map([&](decltype(schema_.all_fields()) o) { f(o); });
  }

  // Update N's members except auto increment members.
  // N must have at least one primary key.
  template <typename N, typename... CB> void update(const N& o, CB&&... args) {
    // check if N has at least one member of PKS.

    call_callback(s::validate, o, args...);
    call_callback(s::write_access, o, args...);
    call_callback(s::before_update, o, args...);

    // static_assert(metamap_size<decltype(intersect(o, schema_.read_only()))>(),
    //"You cannot give read only fields to the orm update method.");

    auto pk = intersection(o, schema_.primary_key());
    static_assert(metamap_size<decltype(pk)>() > 0,
                  "You must provide at least one primary key to update an object.");
    std::ostringstream ss;
    ss << "UPDATE " << schema_.table_name() << " SET ";

    bool first = true;
    auto to_update = substract(o, schema_.read_only());

    map(to_update, [&](auto k, auto v) {
      if (!first)
        ss << ",";
      first = false;
      ss << li::symbol_string(k) << " = ?";
    });

    where_clause(pk, ss);

    auto stmt = con_.prepare(ss.str());
    li::tuple_reduce(std::tuple_cat(metamap_values(to_update), metamap_values(pk)), stmt);

    call_callback(s::after_update, o, args...);
  }

  template <typename A, typename B, typename... O, typename... W>
  void update(metamap<O...>&& o, assign_exp<A, B> w1, W... ws) {
    return update(cat(o, mmm(w1)), ws...);
  }
  template <typename A, typename B, typename... W> void update(assign_exp<A, B> w1, W... ws) {
    return update(mmm(w1), ws...);
  }

  inline int count() {
    return con_(std::string("SELECT count(*) from ") + schema_.table_name()).template read<int>();
  }

  template <typename N, typename... CB> void remove(const N& o, CB&&... args) {

    call_callback(s::before_remove, o, args...);

    std::ostringstream ss;
    ss << "DELETE from " << schema_.table_name() << " WHERE ";

    bool first = true;
    map(schema_.primary_key(), [&](auto k, auto v) {
      if (!first)
        ss << " and ";
      first = false;
      ss << li::symbol_string(k) << " = ? ";
    });

    auto pks = intersection(o, schema_.primary_key());
    li::reduce(pks, con_.prepare(ss.str()));

    call_callback(s::after_remove, o, args...);
  }
  template <typename A, typename B, typename... O, typename... W>
  void remove(metamap<O...>&& o, assign_exp<A, B> w1, W... ws) {
    return remove(cat(o, mmm(w1)), ws...);
  }
  template <typename A, typename B, typename... W> void remove(assign_exp<A, B> w1, W... ws) {
    return remove(mmm(w1), ws...);
  }

  auto& schema() { return schema_; }

  SCHEMA schema_;
  C con_;
};

template <typename... F> struct orm_fields {

  orm_fields(F... fields) : fields_(fields...) {
    static_assert(sizeof...(F) == 0 || metamap_size<decltype(this->primary_key())>() != 0,
                  "You must give at least one primary key to the ORM. Use "
                  "s::your_field_name(s::primary_key) to add a primary_key");
  }

  // Field extractor.
  template <typename M> auto get_field(M m) { return m; }
  template <typename M, typename T> auto get_field(assign_exp<M, T> e) { return e; }
  template <typename M, typename T, typename... A>
  auto get_field(assign_exp<function_call_exp<M, A...>, T> e) {
    return assign_exp<M, T>{M{}, e.right};
  }

  // template <typename M> struct get_field { typedef M ret; };
  // template <typename M, typename T> struct get_field<assign_exp<M, T>> {
  //   typedef assign_exp<M, T> ret;
  //   static auto ctor() { return assign_exp<M, T>{M{}, T()}; }
  // };
  // template <typename M, typename T, typename... A>
  // struct get_field<assign_exp<function_call_exp<M, A...>, T>> : public get_field<assign_exp<M,
  // T>> {
  // };

// get_field<E>::ctor();
// field attributes checks.
#define CHECK_FIELD_ATTR(ATTR)                                                                     \
  template <typename M> struct is_##ATTR : std::false_type {};                                     \
  template <typename M, typename T, typename... A>                                                 \
  struct is_##ATTR<assign_exp<function_call_exp<M, A...>, T>>                                      \
      : std::disjunction<std::is_same<std::decay_t<A>, s::ATTR##_t>...> {};                        \
                                                                                                   \
  auto ATTR() {                                                                                    \
    return tuple_map_reduce(fields_,                                                               \
                            [this](auto e) {                                                       \
                              typedef std::remove_reference_t<decltype(e)> E;                      \
                              if constexpr (is_##ATTR<E>::value)                                   \
                                return get_field(e);                                               \
                              else                                                                 \
                                return skip{};                                                     \
                            },                                                                     \
                            make_metamap_skip);                                                    \
  }

  CHECK_FIELD_ATTR(primary_key);
  CHECK_FIELD_ATTR(read_only);
  CHECK_FIELD_ATTR(auto_increment);
  CHECK_FIELD_ATTR(computed);
#undef CHECK_FIELD_ATTR

  // Do not remove this comment, this is used by the symbol generation.
  // s::primary_key s::read_only s::auto_increment s::computed

  auto all_info() { return fields_; }

  auto all_fields() {

    return tuple_map_reduce(fields_,
                            [this](auto e) {
                              // typedef std::remove_reference_t<decltype(e)> E;
                              return get_field(e);
                            },
                            [](auto... e) { return mmm(e...); });
  }

  auto without_auto_increment() { return substract(all_fields(), auto_increment()); }
  auto all_fields_except_computed() {
    return substract(substract(all_fields(), computed()), auto_increment());
  }

  std::tuple<F...> fields_;
};

template <typename DB, typename MD = orm_fields<>, typename CB = decltype(mmm())>
struct sql_orm_schema : public MD {

  sql_orm_schema(DB& db, const std::string& table_name, CB cb = CB(), MD md = MD())
      : MD(md), database_(db), table_name_(table_name), callbacks_(cb) {}

  inline auto connect() { return sql_orm(*this, database_.connect()); }
  template <typename Y>
  inline auto connect(Y& y) { return sql_orm(*this, database_.connect(y)); }

  const std::string& table_name() const { return table_name_; }
  auto get_callbacks() const { return callbacks_; }

  template <typename... P> auto callbacks(P... params_list) const {
    auto cbs = mmm(params_list...);
    auto allowed_callbacks = mmm(s::before_insert, s::before_remove, s::before_update,
                                 s::after_insert, s::after_remove, s::after_update, s::validate);

    static_assert(
        metamap_size<decltype(substract(cbs, allowed_callbacks))>() == 0,
        "The only supported callbacks are: s::before_insert, s::before_remove, s::before_update,"
        " s::after_insert, s::after_remove, s::after_update, s::validate");
    return sql_orm_schema<DB, MD, decltype(cbs)>(database_, table_name_, cbs,
                                                 *static_cast<const MD*>(this));
  }

  template <typename... P> auto fields(P... p) const {
    return sql_orm_schema<DB, orm_fields<P...>, CB>(database_, table_name_, callbacks_,
                                                    orm_fields<P...>(p...));
  }

  DB& database_;
  std::string table_name_;
  CB callbacks_;
};

}; // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_SQL_SQL_ORM


#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_API
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_API

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_DYNAMIC_ROUTING_TABLE
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_DYNAMIC_ROUTING_TABLE


namespace li {

namespace internal {

template <typename V> struct drt_node {

  drt_node() : v_{0, nullptr} {}
  struct iterator {
    const drt_node<V>* ptr;
    std::string_view first;
    V second;

    auto operator-> () { return this; }
    bool operator==(const iterator& b) const { return this->ptr == b.ptr; }
    bool operator!=(const iterator& b) const { return this->ptr != b.ptr; }
  };

  auto end() const { return iterator{nullptr, std::string_view(), V()}; }

  auto& find_or_create(std::string_view r, unsigned int c) {
    if (c == r.size())
      return v_;

    if (r[c] == '/')
      c++; // skip the /
    int s = c;
    while (c < r.size() and r[c] != '/')
      c++;
    std::string_view k = r.substr(s, c - s);

    auto it = children_.find(k);
    if (it != children_.end())
      return children_[k]->find_or_create(r, c);
    else
    {
      auto new_node = new drt_node();
      children_.insert({k, new_node});
      return new_node->find_or_create(r, c);
    }

    return v_;
  }

  template <typename F> void for_all_routes(F f, std::string prefix = "") const {
    if (children_.size() == 0)
      f(prefix, v_);
    else {
      if (prefix.size() && prefix.back() != '/')
        prefix += '/';
      for (auto pair : children_)
        pair.second->for_all_routes(f, prefix + std::string(pair.first));
    }
  }
  iterator find(const std::string_view& r, unsigned int c) const {
    // We found the route r.
    if ((c == r.size() and v_.handler != nullptr) or (children_.size() == 0))
      return iterator{this, r, v_};

    // r does not match any route.
    if (c == r.size() and v_.handler == nullptr)
      return iterator{nullptr, r, v_};

    if (r[c] == '/')
      c++; // skip the first /

    // Find the next /.
    int s = c;
    while (c < r.size() and r[c] != '/')
      c++;

    // k is the string between the 2 /.
    std::string_view k(&r[s], c - s);

    // look for k in the children.
    auto it = children_.find(k);
    if (it != children_.end()) {
      auto it2 = it->second->find(r, c); // search in the corresponding child.
      if (it2 != it->second->end())
        return it2;
    }

    {
      // if one child is a url param {{param_name}}, choose it
      for (auto& kv : children_) {
        auto name = kv.first;
        if (name.size() > 4 and name[0] == '{' and name[1] == '{' and
            name[name.size() - 2] == '}' and name[name.size() - 1] == '}')
          return kv.second->find(r, c);
      }
      return end();
    }
  }

  V v_;
  std::unordered_map<std::string_view, drt_node*> children_;
};
} // namespace internal

template <typename V> struct dynamic_routing_table {

  // Find a route and return reference to a procedure.
  auto& operator[](const std::string_view& r) {
    strings.push_back(std::shared_ptr<std::string>(new std::string(r)));
    std::string_view r2(*strings.back());
    return root.find_or_create(r2, 0);
  }
  auto& operator[](const std::string& r) {
    strings.push_back(std::shared_ptr<std::string>(new std::string(r)));
    std::string_view r2(*strings.back());
    return root.find_or_create(r2, 0);
  }

  // Find a route and return an iterator.
  auto find(const std::string_view& r) const { return root.find(r, 0); }

  template <typename F> void for_all_routes(F f) const { root.for_all_routes(f); }
  auto end() const { return root.end(); }

  std::vector<std::shared_ptr<std::string>> strings;
  internal::drt_node<V> root;
};

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_DYNAMIC_ROUTING_TABLE

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_ERROR
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_ERROR


namespace li {

template <typename E> inline void format_error_(E&) {}

template <typename E, typename T1, typename... T>
inline void format_error_(E& err, T1&& a, T&&... args) {
  err << a;
  format_error_(err, std::forward<T>(args)...);
}

template <typename... T> inline std::string format_error(T&&... args) {
  std::ostringstream ss;
  format_error_(ss, std::forward<T>(args)...);
  return ss.str();
}

struct http_error {
public:
  http_error(int status, const std::string& what) : status_(status), what_(what) {}
  http_error(int status, const char* what) : status_(status), what_(what) {}
  int status() const { return status_; }
  const std::string& what() const { return what_; }

#define LI_HTTP_ERROR(CODE, ERR)                                                                   \
  template <typename... A> static auto ERR(A&&... m) {                                             \
    return http_error(CODE, format_error(m...));                                                   \
  }                                                                                                \
  static auto ERR(const char* w) { return http_error(CODE, w); }

  LI_HTTP_ERROR(400, bad_request)
  LI_HTTP_ERROR(401, unauthorized)
  LI_HTTP_ERROR(403, forbidden)
  LI_HTTP_ERROR(404, not_found)

  LI_HTTP_ERROR(500, internal_server_error)
  LI_HTTP_ERROR(501, not_implemented)

#undef LI_HTTP_ERROR

private:
  int status_;
  std::string what_;
};

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_ERROR


namespace li {

template <typename T, typename F> struct delayed_assignator {
  delayed_assignator(T& t, F f = [](T& t, auto u) { t = u; }) : t(t), f(f) {}

  template <typename U> auto operator=(U&& u) { return f(t, u); }

  T& t;
  F& f;
};

template <typename Req, typename Resp> struct api {

  enum { ANY, GET, POST, PUT, HTTP_DELETE };

  typedef api<Req, Resp> self;

  using H = std::function<void(Req&, Resp&)>;
  struct VH {
    int verb = ANY;
    H handler;
    std::string url_spec;
  };

  H& operator()(std::string_view& route) {
    auto& vh = routes_map_[route];
    vh.verb = ANY;
    vh.url_spec = route;
    return vh.handler;
  }

  H& operator()(int verb, std::string_view r) {
    auto& vh = routes_map_[r];
    vh.verb = verb;
    vh.url_spec = r;
    return vh.handler;
  }
  H& get(std::string_view r) { return this->operator()(GET, r); }
  H& post(std::string_view r) { return this->operator()(POST, r); }
  H& put(std::string_view r) { return this->operator()(PUT, r); }
  H& delete_(std::string_view r) { return this->operator()(HTTP_DELETE, r); }

  int parse_verb(std::string_view method) const {
    if (method == "GET")
      return GET;
    if (method == "PUT")
      return PUT;
    if (method == "POST")
      return POST;
    if (method == "HTTP_DELETE")
      return HTTP_DELETE;
    return ANY;
  }

  void add_subapi(std::string prefix, const self& subapi) {
    subapi.routes_map_.for_all_routes([this, prefix](auto r, VH h) {
      if (!r.empty() && r.back() == '/')
        h.url_spec = prefix + r;
      else
        h.url_spec = prefix + '/' + r;

      this->routes_map_[h.url_spec] = h;
    });
  }

  void print_routes() {
    routes_map_.for_all_routes([this](auto r, auto h) { std::cout << r << '\n'; });
    std::cout << std::endl;
  }
  auto call(std::string_view method, std::string_view route, Req& request, Resp& response) const {
    // skip the last / of the url.
    std::string_view route2(route);
    if (route2.size() != 0 and route2[route2.size() - 1] == '/')
      route2 = route2.substr(0, route2.size() - 1);

    auto it = routes_map_.find(route2);
    if (it != routes_map_.end()) {
      if (it->second.verb == ANY or parse_verb(method) == it->second.verb) {
        request.url_spec = it->second.url_spec;
        it->second.handler(request, response);
      } else
        throw http_error::not_found("Method ", method, " not implemented on route ", route2);
    } else
      throw http_error::not_found("Route ", route2, " does not exist.");
  }

  dynamic_routing_table<VH> routes_map_;
};

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_API

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_HTTP_LISTEN2
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_HTTP_LISTEN2



#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_MOUSTIQUE
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_MOUSTIQUE
/**
 * @file   moustique.hh
 * @author Matthieu Garrigues <matthieu.garrigues@gmail.com> <matthieu.garrigues@gmail.com>
 * @date   Sat Mar 31 23:52:51 2018
 * 
 * @brief  moustique wrapper.
 * 
 * 
 */



/** 
 * Open a socket on port \port and call \conn_handler(int client_fd, auto read, auto write) 
 * to process each incomming connection. This handle takes 3 argments:
 *               - int client_fd: the file descriptor of the socket.
 *               - int read(buf, max_size_to_read):  
 *                       The callback that conn_handler can use to read on the socket.
 *                       If data is available, copy it into \buf, otherwise suspend the handler until
 *                       there is something to read.
 *                       Returns the number of char actually read, returns 0 if the connection has been lost.
 *               - bool write(buf, buf_size): return true on success, false on error.
 *                       The callback that conn_handler can use to write on the socket.
 *                       If the socket is ready to write, write \buf, otherwise suspend the handler until
 *                       it is ready.
 *                       Returns true on sucess, false if connection is lost.
 *
 * @param port  The server port.
 * @param socktype The socket type, SOCK_STREAM for TCP, SOCK_DGRAM for UDP.
 * @param nthreads Number of threads.
 * @param conn_handler The connection handler
 * @return false on error, true on success.
 */
template <typename H>
int moustique_listen(int port,
                     int socktype,
                     int nthreads,
                     H conn_handler);

// Same as above but take an already opened socket \listen_fd.
template <typename H>
int moustique_listen_fd(int listen_fd,
                        int nthreads,
                        H conn_handler);

namespace moustique_impl
{
  static int create_and_bind(int port, int socktype)
  {
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s, sfd;

    char port_str[20];
    snprintf(port_str, sizeof(port_str), "%d", port);
    memset (&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
    hints.ai_socktype = socktype; /* We want a TCP socket */
    hints.ai_flags = AI_PASSIVE;     /* All interfaces */

    s = getaddrinfo (NULL, port_str, &hints, &result);
    if (s != 0)
    {
      fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (s));
      return -1;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
      sfd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
      if (sfd == -1)
        continue;

      s = bind (sfd, rp->ai_addr, rp->ai_addrlen);
      if (s == 0)
      {
        /* We managed to bind successfully! */
        break;
      }

      close (sfd);
    }

    if (rp == NULL)
    {
      fprintf (stderr, "Could not bind: %s\n", strerror(errno));
      return -1;
    }

    freeaddrinfo (result);

    return sfd;
  }

}

#define MOUSTIQUE_CHECK_CALL(CALL)                                      \
  {                                                                     \
    int ret = CALL;                                                     \
    if (-1 == ret)                                                      \
    {                                                                   \
      fprintf(stderr, "Error at %s:%i  error is: %s\n", __PRETTY_FUNCTION__, __LINE__, strerror(ret)); \
      return false;                                                     \
    }                                                                   \
  }

template <typename H>
int moustique_listen(int port,
                     int socktype,
                     int nthreads,
                     H conn_handler)
{
  return moustique_listen_fd(moustique_impl::create_and_bind(port, socktype), nthreads, conn_handler);
}

static volatile int moustique_exit_request = 0;

static void shutdown_handler(int sig) {
  moustique_exit_request = 1;
  std::cout << "The server will shutdown..." << std::endl;
}

template <typename H>
int moustique_listen_fd(int listen_fd,
                        int nthreads,
                        H conn_handler)
{
  namespace ctx = boost::context;

  if (listen_fd < 0) return 0;
  int flags = fcntl (listen_fd, F_GETFL, 0);
  MOUSTIQUE_CHECK_CALL(fcntl(listen_fd, F_SETFL, flags | O_NONBLOCK));
  MOUSTIQUE_CHECK_CALL(::listen(listen_fd, SOMAXCONN));

  auto event_loop_fn = [listen_fd, conn_handler] () -> int {

    int epoll_fd = epoll_create1(0);

    auto epoll_ctl = [epoll_fd] (int fd, uint32_t flags)
    {
      epoll_event event;
      memset(&event, 0, sizeof(event));
      event.data.fd = fd;
      event.events = flags;
      MOUSTIQUE_CHECK_CALL(::epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event));
      return true;
    };
    auto epoll_ctl_mod = [epoll_fd] (int fd, uint32_t flags)
    {
      epoll_event event;
      event.data.fd = fd;
      event.events = flags;
      MOUSTIQUE_CHECK_CALL(::epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event));
      return true;
    };
    
    epoll_ctl(listen_fd, EPOLLIN | EPOLLET);

    const int MAXEVENTS = 64;
    std::vector<ctx::continuation> fibers;
    std::vector<int> secondary_map;
    // fibers.reserve(1000);
    // Even loop.
    epoll_event events[MAXEVENTS];

    while (!moustique_exit_request)
    {

      int n_events = epoll_wait (epoll_fd, events, MAXEVENTS, 1000);
      if (moustique_exit_request) 
        break;


      for (int i = 0; i < n_events; i++)
      {

        if ((events[i].events & EPOLLERR) ||
            (events[i].events & EPOLLHUP))
        {
          close (events[i].data.fd);
          fibers[events[i].data.fd] = fibers[events[i].data.fd].resume();
          continue;
        }
        else if (listen_fd == events[i].data.fd) // New connection.
        {
          while(true)
          {
            struct sockaddr in_addr;
            socklen_t in_len;
            int infd;

            in_len = sizeof in_addr;
            infd = accept (listen_fd, &in_addr, &in_len);
            if (infd == -1)
              break;

            MOUSTIQUE_CHECK_CALL(fcntl(infd, F_SETFL, fcntl(infd, F_GETFL, 0) | O_NONBLOCK));
            epoll_ctl(infd, EPOLLIN | EPOLLOUT | EPOLLET);
            auto listen_to_new_fd = [original_fd=infd,epoll_ctl,&secondary_map] (int new_fd) {
              if (new_fd > secondary_map.size() || secondary_map[new_fd] == -1)
                epoll_ctl(new_fd, EPOLLIN | EPOLLOUT | EPOLLET);
              if (int(secondary_map.size()) < new_fd + 1)
                secondary_map.resize(new_fd + 10, -1);             
              secondary_map[new_fd] = original_fd;
            };

            if (int(fibers.size()) < infd + 1)
              fibers.resize(infd + 10);

            struct end_of_file {};
            fibers[infd] = ctx::callcc([fd=infd, &conn_handler, epoll_ctl_mod, listen_to_new_fd]
                                       (ctx::continuation&& sink) {
                                         //ctx::continuation sink = std::move(_sink);
                                         auto read = [fd, &sink, epoll_ctl_mod] (char* buf, int max_size) {
                                           ssize_t count = ::recv(fd, buf, max_size, 0);
                                           while (count <= 0)
                                           {
                                             if ((count < 0 and errno != EAGAIN) or count == 0)
                                               return ssize_t(0);
                                             sink = sink.resume();
                                             count = ::recv(fd, buf, max_size, 0);
                                           }
                                           return count;
                                         };

                                         auto write = [fd, &sink, epoll_ctl_mod] (const char* buf, int size) {
                                           if (!buf or !size)
                                           {
                                             //std::cout << "pause" << std::endl;
                                             sink = sink.resume();
                                             return true;
                                           }
                                           const char* end = buf + size;
                                           ssize_t count = ::send(fd, buf, end - buf, MSG_NOSIGNAL);
                                           if (count > 0) buf += count;
                                           while (buf != end)
                                           {
                                             if ((count < 0 and errno != EAGAIN) or count == 0)
                                               return false;
                                             sink = sink.resume();
                                             count = ::send(fd, buf, end - buf, MSG_NOSIGNAL);
                                             if (count > 0) buf += count;
                                           }
                                           return true;
                                         };
              
                                         conn_handler(fd, read, write, listen_to_new_fd);
                                         close(fd);
                                         return std::move(sink);
                                       });
          
          }
          
        }
        else // Data available on existing sockets. Wake up the fiber associated with events[i].data.fd.
        {

          if (events[i].data.fd < secondary_map.size() && secondary_map[events[i].data.fd] != -1)
          {
            int original_fd = secondary_map[events[i].data.fd];
            if (fibers[original_fd])
              fibers[original_fd] = fibers[original_fd].resume();
          }
          else
            if (fibers[events[i].data.fd])
              fibers[events[i].data.fd] = fibers[events[i].data.fd].resume();
        }
      }
    }
    close(epoll_fd);
    return true;  
  };

  struct sigaction act;
    memset (&act, 0, sizeof(act));
	  act.sa_handler = shutdown_handler;
  
  sigaction(SIGINT, &act, 0);
  sigaction(SIGTERM, &act, 0);
  sigaction(SIGQUIT, &act, 0);
  
  std::vector<std::thread> ths;
  for (int i = 0; i < nthreads; i++)
    ths.push_back(std::thread([&] { event_loop_fn(); }));

  for (auto& t : ths)
    t.join();

  close(listen_fd);

  return true;
}

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_MOUSTIQUE

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_URL_UNESCAPE
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_URL_UNESCAPE


namespace li {
std::string_view url_unescape(std::string_view str) {
  char* o = (char*)str.data();
  char* c = (char*)str.data();
  const char* end = c + str.size();

  while (c < end) {
    if (*c == '%' && c < end - 2) {
      char number[3];
      number[0] = c[1];
      number[1] = c[2];
      number[2] = 0;
      *o = char(strtol(number, nullptr, 16));
      c += 2;
    } else if (o != c)
      *o = *c;
    o++;
    c++;
  }
  return std::string_view(str.data(), o - str.data());
}
} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_URL_UNESCAPE


namespace li {  

namespace http_async_impl {  

char* date_buf = nullptr;
int date_buf_size = 0;

thread_local std::unordered_map<std::string, std::string_view> static_files;

struct read_buffer
{

  std::array<char, 20*1024> buffer_;
  //std::vector<char> buffer_;
  int cursor = 0; // First index of the currently used buffer area
  int end = 0; // Index of the last read character
  
  read_buffer()
    : //buffer_(20 * 1024),
      cursor(0),
      end(0)
  {}

  // Free unused space in the buffer in [i1, i2[.
  // This may move data in [i2, end[ if needed.
  // Return the
  void free(int i1, int i2)
  {
    assert(i1 < i2);
    assert(i1 >= 0 and i1 < buffer_.size());
    assert(i2 > 0 and i2 <= buffer_.size());

    if (i1 == cursor and i2 == end) // eat the whole buffer.
      cursor = end = 0;
    else if (i1 == cursor) // eat the beggining of the buffer.
      cursor = i2;
    else if (i2 == end) end = i1; // eat the end of the buffer.
    else if (i2 != end) // eat somewhere in the middle.
    {
      if (buffer_.size() - end < buffer_.size() / 4)
      {
        if (end - i2 > i2 - i1) // use memmove if overlap.
          std::memmove(buffer_.data() + i1, buffer_.data() + i2, end - i2);
        else
          std::memcpy(buffer_.data() + i1, buffer_.data() + cursor, end - cursor);
      }
    }
  }

  void free(const char* i1, const char* i2) { 
    assert(i1 >= buffer_.data());
    assert(i1 <= &buffer_.back());
    assert(i2 >= buffer_.data() and i2 <= &buffer_.back());
    free(i1 - buffer_.data(), i2 - buffer_.data()); 
  }
  void free(const std::string_view& str) { free(str.data(), str.data() + str.size()); }
  
  // private: Read more data
  // Read from ptr until character x.
  // read n more characters at address ptr.
  // eat n first/last character.
  // free part of the buffer and more data if needed.

  // Read more data.
  // Return 0 on error.
  template <typename F>
  int read_more(F&& read, int size = -1)
  {
    if (int(buffer_.size()) <= end - 100)
    {
      // reset buffer_.
      cursor = 0; end = 0; 
      throw std::runtime_error("Request too long.");

      // if (buffer_.size() > (10*1024*1024))
      //   return 0; // Buffer is full. Error and return.
      // else 
      {
        // std::cout << "RESIZE" << std::endl;
        // buffer_.resize(buffer_.size() * 2);
      }
    }

    if (size == -1) size = buffer_.size() - end;
    int received = read(buffer_.data() + end, size);
    if (received == 0) return 0; // Socket closed, return.
    end = end + received;
    //std::cout << "read size " << end << std::endl;
    return received;
  }
  template <typename F>
  std::string_view read_more_str(F&& read)
  {
    int l = read_more(read);
    return std::string_view(buffer_.data() + end - l);
  }

  template <typename F>
  std::string_view read_n(F&& read, const char* start, int size)
  {
    int str_start = start - buffer_.data();
    int str_end = size + str_start;
    if (end < str_end)
    {
      // Read more body on the socket.
      int current_size = end - str_start;
      while (current_size < size)
          current_size += read_more(read);
    }
    return std::string_view(start, size);
  }
  
  template <typename F>
  std::string_view read_until(F&& read, const char*& start, char delimiter)
  {
    const char* str_end = start;

    while (true)
    {
      const char* buffer_end = buffer_.data() + end;
      while (str_end < buffer_end and *str_end != delimiter) str_end++;

      if (*str_end == delimiter) break;
      else {
        if (!read_more(read)) break;
      }
    }

    auto res = std::string_view(start, str_end - start);
    start = str_end + 1;
    return res;
  }

  bool empty() const { return cursor == end; }

  // Return the amount of data currently available to read.
  int current_size() { return end - cursor; }

  // Reset the buffer. Copy remaining data at the beggining if there is some.
  void reset() {
    assert(cursor <= end);
    if (cursor == end)
      end = cursor = 0;
    else
    {
      if (cursor > end - cursor) // use memmove if overlap.
        std::memmove(buffer_.data(), buffer_.data() + cursor, end - cursor);
      else
        std::memcpy(buffer_.data(), buffer_.data() + cursor, end - cursor);

      // if (
      // memcpy(buffer_.data(), buffer_.data() + cursor, end - cursor);

      end = end - cursor;
      cursor = 0;
    }

  }

  // On success return the number of bytes read.
  // On error return 0.
  char* data() { return buffer_.data(); }
};

struct output_buffer
{

  output_buffer(void* buffer, int capacity)
    : buffer_((char*)buffer),
      cursor_(buffer_),
      end_(buffer_ + capacity)      
  {
    assert(buffer_);
  }

  void reset() 
  {
    cursor_ = buffer_;
  }

  std::size_t size()
  {
    return cursor_ - buffer_;
  }

  output_buffer& operator<<(std::string_view s)
  {
    if (cursor_ + s.size() >= end_)
      throw std::runtime_error("Response too long.");
    assert(cursor_ + s.size() < end_);
    memcpy(cursor_, s.data(), s.size());
    cursor_ += s.size();
    return *this;
  }
  output_buffer& operator<<(char v)
  {
    cursor_[0] = v;
    cursor_++;
    return *this;
  }

  // output_buffer& operator<<(int v) { return integer(v); }
  // output_buffer& operator<<(unsigned int v) { return integer(v); }
  // output_buffer& operator<<(size_t v) { return integer(v); }

  template <typename I>
  output_buffer& operator<<(I v)
  {
    typedef std::array<char, 50> buf_t;
    buf_t b = boost::lexical_cast<buf_t>(v);
    operator<<(std::string_view(b.begin(), strlen(b.begin())));
    return *this;
  }
  
  std::string_view to_string_view() { return std::string_view(buffer_, cursor_ - buffer_); }
  char* buffer_;
  char* cursor_;
  char* end_;
};

struct http_ctx {

  http_ctx(read_buffer& _rb,
           std::function<int(char*, int)> _read,
           std::function<bool(const char*, int)> _write,
           std::function<void(int)> _listen_to_new_fd
           )
    : rb(_rb),
      read(_read),
      write(_write),
      listen_to_new_fd(_listen_to_new_fd),
      headers_stream(headers_buffer_space, sizeof(headers_buffer_space)),
      //output_buffer_space(new char[1000 * 1024]),
      output_stream(output_buffer_space, sizeof(output_buffer_space))
  {
    get_parameters_map.reserve(10);
    response_headers.reserve(20);
  }
  //~http_ctx() { delete[] output_buffer_space; }

  http_ctx& operator=(const http_ctx&) = delete;
  http_ctx(const http_ctx&) = delete;

  std::string_view header(const char* key)
  {
    if (!header_map.size())
      index_headers();
    return header_map[key];
  }
  
  std::string_view cookie(const char* key)
  {
    if (!cookie_map.size())
      index_cookies();
    return cookie_map[key];
  }

  std::string_view get_parameter(const char* key)
  {
    if (!url_.size())
      parse_first_line();
    return get_parameters_map[key];
  }
  
  // std::string_view post_parameter(const char* key)
  // {
  //   if (!is_body_read_)
  //   {
  //     //read_whole_body();
  //     parse_post_parameters();
  //   }
  //   return post_parameters_map[key];
  // }
 
  // std::string_view read_body(char* buf, int buf_size)
  // {
  //   // Try to read Content-Length

  //   // if part of the body is in the request buffer
  //   //  return it and mark it as read.
  //   // otherwise call read on the socket.

  //   // handle chunked encoding here if needed.
  // }

  // void sendfile(std::string path)
  // {
  //   char buffer[10200];
  //   //std::cout << uint64_t(output_stream.buffer_) << " " << uint64_t(output_buffer_space) << std::endl;
  //   output_buffer output_stream(buffer, sizeof(buffer));

  //   struct stat stat_buf;
  //   int read_fd = open (path.c_str(), O_RDONLY);
  //   fstat (read_fd, &stat_buf);


  //   format_top_headers(output_stream);
  //   output_stream << headers_stream.to_string_view();
  //   output_stream << "Content-Length: " << size_t(stat_buf.st_size) << "\r\n\r\n"; //Add body
  //   auto m = output_stream.to_string_view();
  //   write(m.data(), m.size());
    
  //   off_t offset = 0;
  //   while (true)
  //   {
  //     int ret = ::sendfile(socket_fd, read_fd, &offset, stat_buf.st_size);
  //     if (ret == EAGAIN)
  //       write(nullptr, 0);
  //     else break;
  //   }
  // }

  std::string_view url()
  {
    if (!url_.size())
      parse_first_line();
    return url_;
  }
  std::string_view method()
  {
    if (!method_.size())
      parse_first_line();
    return method_;
  }
  std::string_view http_version()
  {
    if (!url_.size())
      parse_first_line();
    return http_version_;
  }

  inline void format_top_headers(output_buffer& output_stream)
  {
    output_stream << "HTTP/1.1 " << status_ << "\r\n";
    output_stream << "Date: " << std::string_view(date_buf, date_buf_size) << "\r\n";
    output_stream << "Connection: keep-alive\r\nServer: Moustique\r\n";
  }
  
  void prepare_request()
  {
    //parse_first_line();
    response_headers.clear();
    content_length_ = 0;
    chunked_ = 0;
    
    for (int i = 1; i < header_lines_size - 1; i++)
    {
      const char* line_end = header_lines[i + 1]; // last line is just an empty line.
      const char* cur = header_lines[i];

      if (*cur != 'C' and *cur != 'c') continue;

      std::string_view key = split(cur, line_end, ':');

      auto get_value = [&] {
        std::string_view value = split(cur, line_end, '\r');
        while (value[0] == ' ')
          value = std::string_view(value.data() + 1, value.size() - 1);
        return value;
      };

      if (key == "Content-Length")
        content_length_ = atoi(get_value().data());
      else if (key == "Content-Type")
      {
        content_type_ = get_value();
        chunked_ = (content_type_ == "chunked");
      }
      
      // header_map[key] = value;
      // std::cout << key << " -> " << value << std::endl;
    }

  }

  //void respond(std::string s) {return respond(std::string_view(s)); }

  // void respond(const char* s)
  // {
  //   return respond(std::string_view(s, strlen(s)));
  // }

  void respond(const std::string_view& s)
  {
    response_written_ = true;
    if (s.size() > 10000) // writev for large content.
    {
      format_top_headers(output_stream);
      output_stream << headers_stream.to_string_view();
      output_stream << "Content-Length: " << s.size() << "\r\n\r\n"; //Add body

      auto m = output_stream.to_string_view();
      iovec iov[2];
      iov[0].iov_base = (char*)m.data();
      iov[0].iov_len = m.size();
      iov[1].iov_base = (char*)s.data();
      iov[1].iov_len = s.size();

      int total_size = 0;
      for (int i = 0; i < 1; i++) total_size += iov[i].iov_len;

      int ret = 0;
      do
      {
        ret = writev(socket_fd, iov, 2);
        if (ret == -1 and errno == EAGAIN)
          write(nullptr, 0);// yield
        assert(ret < 0 or ret == total_size);
      } while (ret == -1 and errno == EAGAIN);
    }
    else // write for small content.
     {
      format_top_headers(output_stream);
      output_stream << headers_stream.to_string_view();
      output_stream << "Content-Length: " << s.size() << "\r\n\r\n" << s; //Add body
      //std::cout << output_stream.to_string_view().size() << std::endl;
      //flush_responses();
      //auto m = output_stream.to_string_view();
      //write(m.data(), m.size());

      // const char* resp = "HTTP/1.1 200 OK\r\nServer: lithium\r\nDate: Fri, 13 Dec 2019 21:46:28 GMT\r\nContent-Type: Text/Plain\r\nContent-Length: 12\r\n\r\nhello world!";
      // write(resp, strlen(resp));
    }
  }

  template <typename O>
  void respond_json(const O& obj)
  {
    response_written_ = true;
    char json_buffer[10000];
    output_buffer json_stream(json_buffer, sizeof(json_buffer));

    json_encode(json_stream, obj);

    format_top_headers(output_stream);
    output_stream << headers_stream.to_string_view();
    output_stream << "Content-Length: " << json_stream.to_string_view().size() << "\r\n\r\n"
                  << json_stream.to_string_view();
    //auto m = output_stream.to_string_view();
    //write(m.data(), m.size());
  }
  

  void respond_if_needed()
  {
    if (!response_written_)
    {
      response_written_ = true;

      format_top_headers(output_stream);
      output_stream << headers_stream.to_string_view();
      output_stream << "Content-Length: 0\r\n\r\n"; //Add body
      //auto m = output_stream.to_string_view();
      //write(m.data(), m.size());
    }
  }

  
  void set_header(std::string_view k, std::string_view v) { 
    headers_stream << k << ": " << v << "\r\n"; 
  }

  void set_cookie(std::string_view k, std::string_view v) { 
    headers_stream << "Set-Cookie: " << k << '=' << v << "\r\n";
  }

  void set_status(int status) {

    switch (status)
    {
    case 200: status_ = "200 OK"; break;
    case 201: status_ = "201 Created"; break;
    case 204: status_ = "204 No Content"; break;
    case 304: status_ = "304 Not Modified"; break;
    case 400: status_ = "400 Bad Request"; break;
    case 401: status_ = "401 Unauthorized"; break;
    case 402: status_ = "402 Not Found"; break;
    case 403: status_ = "403 Forbidden"; break;
    case 404: status_ = "404 Not Found"; break;
    case 409: status_ = "409 Conflict"; break;
    case 500: status_ = "500 Internal Server Error"; break;
    default: status_ = "200 OK"; break;
    }
    
  }
  
  void send_static_file(const char* path)
  {
    auto it = static_files.find(path);
    if (static_files.end() == it or !it->second.size())
    {
      int fd = open(path, O_RDONLY);
      if (fd == -1)
        throw http_error::not_found("File not found.");
      int file_size = lseek(fd, (size_t)0, SEEK_END);
      auto content = std::string_view((char*)mmap(0, file_size, PROT_READ, MAP_SHARED, fd, 0), file_size);
      static_files.insert({path, content});
      respond(content);
    }
    else
      respond(it->second);
  }
  
  //private:

  void add_header_line(const char* l) { header_lines[header_lines_size++] = l; }
  const char* last_header_line() { return header_lines[header_lines_size - 1]; }

  // split a string, starting from cur and ending with split_char.
  // Advance cur to the end of the split.
  std::string_view split(const char*& cur,
                         const char* line_end, char split_char)
  {

    const char* start = cur;
    while (start < (line_end-1) and *start == split_char) start++;
    const char* end = start + 1;
    while (end < (line_end-1) and *end != split_char) end++;
    cur = end + 1;
    if (*end == split_char)
      return std::string_view(start, cur - start - 1);
    else
      return std::string_view(start, cur - start);
  }
  
  void index_headers()
  {
    for (int i = 1; i < header_lines_size - 1; i++)
    {
      const char* line_end = header_lines[i + 1]; // last line is just an empty line.
      const char* cur = header_lines[i];

      std::string_view key = split(cur, line_end, ':');
      std::string_view value = split(cur, line_end, '\r');
      while (value[0] == ' ')
        value = std::string_view(value.data() + 1, value.size() - 1);
      header_map[key] = value;
      //std::cout << key << " -> " << value << std::endl;
    }
  }

  void index_cookies()
  {
    std::string_view cookies = header("Cookie");
    if (!cookies.data()) return;
    const char* line_end = &cookies.back() + 1;
    const char* cur = &cookies.front();
    while(cur < line_end)
    {

      std::string_view key = split(cur, line_end, '=');
      std::string_view value = split(cur, line_end, ';');
      while (key[0] == ' ')
        key = std::string_view(key.data() + 1, key.size() - 1);
      cookie_map[key] = value;
    }
  }

  template <typename C>
  void url_decode_parameters(std::string_view content, C kv_callback)
  {
    const char* c = content.data();
    const char* end = c + content.size();
    if (c < end)
    {
      while (c < end)
      {
        std::string_view key = split(c, end, '=');
        std::string_view value = split(c, end, '&');
        kv_callback(key, value);
        // printf("kv: '%.*s' -> '%.*s'\n", key.size(), key.data(), value.size(), value.data());
      }
    }    
  }
    
  void parse_first_line()
  {    
    const char* c = header_lines[0];
    const char* end = header_lines[1];

    method_ = split(c, end, ' ');
    url_ = split(c, end, ' ');
    http_version_ = split(c, end, '\r');

    // url get parameters.
    c = url_.data();
    end = c + url_.size();
    url_ = split(c, end, '?');
    get_parameters_string_ = std::string_view(c, end - c);

  }

  std::string_view get_parameters_string()
  {
    if (!get_parameters_string_.data())
      parse_first_line();
    return get_parameters_string_;
  }
  template <typename F>
  void parse_get_parameters(F processor)
  {
    url_decode_parameters(get_parameters_string(), processor);
  }

  template <typename F>
  void read_body(F callback)
  {
    is_body_read_ = true;

    if (!chunked_ and !content_length_)
      body_end_ = body_start.data();
    else if (content_length_)
    {
      std::string_view res;
      int n_body_read = 0;

      // First return part of the body already in memory.
      int l = std::min(int(body_start.size()), content_length_);
      callback(std::string_view(body_start.data(), l));
      n_body_read += l;
      
      while (content_length_ > n_body_read)
      {
        std::string_view part = rb.read_more_str(read);
        int l = part.size();
        int bl = std::min(l, content_length_ - n_body_read);
        part = std::string_view(part.data(), bl);
        callback(part);
        rb.free(part);
        body_end_ = part.data();
        n_body_read += part.size();
      }

    }
    else if (chunked_)
    {
      // Chunked decoding.
      const char* cur = body_start.data();
      int chunked_size = strtol(rb.read_until(read, cur, '\r').data(), nullptr, 16);
      cur++; // skip \n
      while (chunked_size > 0)
      {
        // Read chunk.
        std::string_view chunk = rb.read_n(read, cur, chunked_size);
        callback(chunk);
        rb.free(chunk);
        cur += chunked_size + 2; // skip \r\n.

        // Read next chunk size.
        chunked_size = strtol(rb.read_until(read, cur, '\r').data(), nullptr, 16);
        cur++; // skip \n
      }
      cur += 2;// skip the terminaison chunk.
      body_end_ = cur;
      body_ = std::string_view(body_start.data(), cur - body_start.data());
    }
  }
  
  std::string_view read_whole_body()
  {
    if (!chunked_ and !content_length_)
    {
      is_body_read_ = true;
      body_end_ = body_start.data();
      return std::string_view(); // No body.
    }

    if (content_length_)
    {
      body_ = rb.read_n(read, body_start.data(), content_length_);
      body_end_ = body_.data() + content_length_;
    }
    else if (chunked_)
    {
      // Chunked decoding.
      char* out = (char*) body_start.data();
      const char* cur = body_start.data();
      int chunked_size = strtol(rb.read_until(read, cur, '\r').data(), nullptr, 16);
      cur++; // skip \n
      while (chunked_size > 0)
      {
        // Read chunk.
        std::string_view chunk = rb.read_n(read, cur, chunked_size);
        cur += chunked_size + 2; // skip \r\n.
        // Copy the body into a contiguous string.
        if (out + chunk.size() > chunk.data()) // use memmove if overlap.
          std::memmove(out, chunk.data(), chunk.size());
        else
          std::memcpy(out, chunk.data(), chunk.size());
        
        out += chunk.size();

        // Read next chunk size.
        chunked_size = strtol(rb.read_until(read, cur, '\r').data(), nullptr, 16);
        cur++; // skip \n
      }
      cur += 2;// skip the terminaison chunk.
      body_end_ = cur;
      body_ = std::string_view(body_start.data(), out - body_start.data());
    }

    is_body_read_ = true;    
    return body_;
  }

  void read_multipart_formdata()
  {

  }

  template <typename F>
  void post_iterate(F kv_callback)
  {
    if (is_body_read_) // already in memory.
      url_decode_parameters(body_, kv_callback);
    else // stream the body.
    {
      std::string_view current_key;

      read_body([] (std::string_view part) {
          // read key if needed.
          //if (!current_key.size())
          // call kv callback if some value is available.          
        });
    }
  }
  
  // Read post parameters in the body.
  std::unordered_map<std::string_view, std::string_view> post_parameters()
  {
    if (content_type_ == "application/x-www-form-urlencoded")
    {
      if (!is_body_read_)
        read_whole_body();
      url_decode_parameters(body_, [&] (auto key, auto value)
                            { post_parameters_map[key] = value; });
      return post_parameters_map;
    }
    else
    {
      // fixme: return bad request here.
    }
    return post_parameters_map;
  }

  void prepare_next_request()
  {
    if (!is_body_read_)
      read_whole_body();

    // std::cout <<"free line0: " << uint64_t(header_lines[0]) << std::endl;
    // std::cout << rb.current_size() << " " << rb.cursor << std::endl;
    rb.free(header_lines[0], body_end_);
    // std::cout << rb.current_size() << " " << rb.cursor << std::endl;
    //rb.cursor = rb.end = 0;
    //assert(rb.cursor == 0);
    headers_stream.reset();
    status_ = "200 OK";
    method_ = std::string_view();
    url_ = std::string_view();
    http_version_ = std::string_view();
    content_type_ = std::string_view();
    header_map.clear();
    cookie_map.clear();
    response_headers.clear();
    get_parameters_map.clear();
    post_parameters_map.clear();
    get_parameters_string_ = std::string_view();
    response_written_ = false;
  }

  void flush_responses() {
    auto m = output_stream.to_string_view();
    if (m.size() > 0)
    {
      write(m.data(), m.size());
      output_stream.reset();
    }
  }

  int socket_fd;
  read_buffer& rb;

  const char* status_ = "200 OK";
  std::string_view method_;
  std::string_view url_;
  std::string_view http_version_;
  std::string_view content_type_;
  bool chunked_;
  int content_length_;
  std::unordered_map<std::string_view, std::string_view> header_map;
  std::unordered_map<std::string_view, std::string_view> cookie_map;
  std::vector<std::pair<std::string_view, std::string_view>> response_headers;
  std::unordered_map<std::string_view, std::string_view> get_parameters_map;
  std::unordered_map<std::string_view, std::string_view> post_parameters_map;
  std::string_view get_parameters_string_;
  //std::vector<std::string> strings_saver;

  bool is_body_read_ = false;
  std::string body_local_buffer_;
  std::string_view body_;
  std::string_view body_start;
  const char* body_end_ = nullptr;
  const char* header_lines[100];
  int header_lines_size = 0;
  std::function<bool(const char*, int)> write;
  std::function<int(char*, int)> read;
  std::function<void(int)> listen_to_new_fd;
  char headers_buffer_space[1000];
  output_buffer headers_stream;
  bool response_written_ = false;

  char output_buffer_space[10*1024];
  //char* output_buffer_space;
  output_buffer output_stream;
}; 

template <typename F>
auto make_http_processor(F handler)
{
  return [handler] (int fd, auto read, auto write, auto listen_to_new_fd) {

    try {
      read_buffer rb;
      bool socket_is_valid = true;

      //http_ctx& ctx = *new http_ctx(rb, read, write);
      http_ctx ctx = http_ctx(rb, read, write, listen_to_new_fd);
      ctx.socket_fd = fd;
      //ctx.header_lines = new const char*[10];
      
      while (true)
      {
        ctx.is_body_read_ = false;
        ctx.header_lines_size = 0;
        // Read until there is a complete header.
        int header_start = rb.cursor;
        int header_end = rb.cursor;
        assert(header_start >= 0);
        assert(header_end >= 0);
        assert(ctx.header_lines_size == 0);
        ctx.add_header_line(rb.data() + header_end);
        //std::cout <<"set line0: " << uint64_t(rb.data() + header_end) << std::endl;
        assert(ctx.header_lines_size == 1);

        bool complete_header = false;
        while (!complete_header)
        {
          // Read more data from the socket.
          if (rb.empty())
            if (!rb.read_more(read)) return;

          // Look for end of header and save header lines.
          while (header_end < rb.end - 3)
          {
            //if (!strncmp(rb.data() + header_end, "\r\n", 2))
            if ((rb.data() + header_end)[0] == '\r' and (rb.data() + header_end)[1] == '\n')
            {
              ctx.add_header_line(rb.data() + header_end + 2);
              header_end += 2;
              if ((rb.data() + header_end)[0] == '\r' and (rb.data() + header_end)[1] == '\n')
                //if (!strncmp(rb.data() + header_end, "\r\n", 2))
              {
                complete_header = true;
                header_end += 2;
                break;
              }
            }
            else           
              header_end++;
          }
        }

        // Header is complete. Process it.
        // Run the handler.
        assert(rb.cursor <= rb.end);
        ctx.body_start = std::string_view(rb.data() + header_end, rb.end - header_end);
        ctx.prepare_request();
        handler(ctx);
        //delete[] ctx.header_lines;
        assert(rb.cursor <= rb.end);

        // Update the cursor the beginning of the next request.
        ctx.prepare_next_request();
        // if read buffer is empty, we can flush the output buffer.
        //std::cout << rb.current_size() << " " << rb.empty() << std::endl;
        if (rb.empty())// || ctx.output_stream.size() > 100000)
        ctx.flush_responses();

      }
      // printf("conection lost %d.\n", fd);
      //delete &ctx;
    }
    catch (const std::runtime_error& e) 
    {
      std::cerr << "Error: " << e.what() << std::endl;
      return;
    }
  };

}

} // namespace http_async_impl
}


#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_REQUEST
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_REQUEST


#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_URL_DECODE
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_URL_DECODE

#if defined(_MSC_VER)
#endif


namespace li {
// Decode a plain value.
template <typename O>
std::string_view url_decode2(std::set<void*>& found, std::string_view str, O& obj) {
  if (str.size() == 0)
    throw std::runtime_error(format_error("url_decode error: expected key end"));

  if (str[0] != '=')
    throw std::runtime_error(format_error("url_decode error: expected =, got ", str[0]));

  int start = 1;
  int end = 1;

  while (str.size() != end && str[end] != '&')
    end++;

  if (start != end) {
    std::string content(str.substr(start, end - start));
    // Url escape.
    //content.resize(MHD_http_unescape(&content[0]));
    if constexpr(std::is_same<std::remove_reference_t<decltype(obj)>, std::string>::value or
                std::is_same<std::remove_reference_t<decltype(obj)>, std::string_view>::value)
      obj = content;
    else
      obj = boost::lexical_cast<O>(content);
    found.insert(&obj);
  }
  if (end == str.size())
    return str.substr(end, 0);
  else
    return str.substr(end + 1, str.size() - end - 1);
}

template <typename O>
std::string_view url_decode2(std::set<void*>& found, std::string_view str, std::optional<O>& obj) {
  O o;
  auto ret = url_decode2(found, str, o);
  obj = o;
  return ret;
}

template <typename... O>
std::string_view url_decode2(std::set<void*>& found, std::string_view str, metamap<O...>& obj,
                             bool root = false);

// Decode an array element.
template <typename O>
std::string_view url_decode2(std::set<void*>& found, std::string_view str, std::vector<O>& obj) {
  if (str.size() == 0)
    throw std::runtime_error(format_error("url_decode error: expected key end", str[0]));

  if (str[0] != '[')
    throw std::runtime_error(format_error("url_decode error: expected [, got ", str[0]));

  // Get the index substring.
  int index_start = 1;
  int index_end = 1;

  while (str.size() != index_end and str[index_end] != ']')
    index_end++;

  if (str.size() == 0)
    throw std::runtime_error(format_error("url_decode error: expected key end", str[0]));

  auto next_str = str.substr(index_end + 1, str.size() - index_end - 1);

  if (index_end == index_start) // [] syntax, push back a value.
  {
    O x;
    auto ret = url_decode2(found, next_str, x);
    obj.push_back(x);
    return ret;
  } else // [idx] set index idx.
  {
    int idx = std::strtol(str.data() + index_start, nullptr, 10);
    if (idx >= 0 and idx <= 9999) {
      if (int(obj.size()) <= idx)
        obj.resize(idx + 1);
      return url_decode2(found, next_str, obj[idx]);
    } else
      throw std::runtime_error(format_error("url_decode error: out of bound array subscript."));
  }
}

// Decode an object member.
template <typename... O>
std::string_view url_decode2(std::set<void*>& found, std::string_view str, metamap<O...>& obj,
                             bool root) {
  if (str.size() == 0)
    throw http_error::bad_request("url_decode error: expected key end", str[0]);

  int key_start = 0;
  int key_end = key_start + 1;

  int next = 0;

  if (!root) {
    if (str[0] != '[')
      throw std::runtime_error(format_error("url_decode error: expected [, got ", str[0]));

    key_start = 1;
    while (key_end != str.size() && str[key_end] != ']' && str[key_end] != '=')
      key_end++;

    if (key_end == str.size())
      throw std::runtime_error("url_decode error: unexpected end.");
    next = key_end + 1; // skip the ]
  } else {
    while (key_end < str.size() && str[key_end] != '[' && str[key_end] != '=')
      key_end++;
    next = key_end;
  }

  auto key = str.substr(key_start, key_end - key_start);
  auto next_str = str.substr(next, str.size() - next);

  std::string_view ret;
  map(obj, [&](auto k, auto& v) {
    if (li::symbol_string(k) == key) {
      try {
        ret = url_decode2(found, next_str, v);
      } catch (std::exception e) {
        throw std::runtime_error(
            format_error("url_decode error: cannot decode parameter ", li::symbol_string(k)));
      }
    }
  });
  return ret;
}

template <typename O>
std::string url_decode_check_missing_fields(const std::set<void*>& found, std::optional<O>& obj) {
  return std::string();
}

template <typename O>
std::string url_decode_check_missing_fields(const std::set<void*>& found, O& obj) {
  if (found.find(&obj) == found.end())
    return " ";
  else
    return std::string();
}

template <typename O>
std::string url_decode_check_missing_fields(const std::set<void*>& found, std::vector<O>& obj) {
  // Fixme : implement missing fields checking in std::vector<metamap<O...>>
  // for (int i = 0; i < obj.size(); i++)
  // {
  //   std::string missing = url_decode_check_missing_fields(found, obj[i]);
  //   if (missing.size())
  //   {
  //     std::ostringstream ss;
  //     ss << '[' << i << "]" << missing;
  //     return ss.str();
  //   }
  // }
  return std::string();
}

template <typename... O>
std::string url_decode_check_missing_fields(const std::set<void*>& found, metamap<O...>& obj,
                                            bool root = false) {
  std::string missing;
  map(obj, [&](auto k, auto& v) {
    if (missing.size())
      return;
    missing = url_decode_check_missing_fields(found, v);
    if (missing.size())
      missing = (root ? "" : ".") + std::string(li::symbol_string(k)) + missing;
  });
  return missing;
}

template <typename S, typename... O>
std::string url_decode_check_missing_fields_on_subset(const std::set<void*>& found,
                                                      metamap<O...>& obj, bool root = false) {
  std::string missing;
  map(S(), [&](auto k, auto) {
    if (missing.size())
      return;
    auto& v = obj[k];
    missing = url_decode_check_missing_fields(found, v);
    if (missing.size())
      missing = (root ? "" : ".") + std::string(li::symbol_string(k)) + missing;
  });
  return missing;
}

// Frontend.
template <typename O> void url_decode(std::string_view str, O& obj) {
  std::set<void*> found;

  // Parse the urlencoded string
  while (str.size() > 0)
    str = url_decode2(found, str, obj, true);

  // Check for missing fields.
  std::string missing = url_decode_check_missing_fields(found, obj, true);
  if (missing.size())
    throw std::runtime_error(format_error("Missing argument ", missing));
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_URL_DECODE

//#include <li/http_backend/http_listen2.hh>



namespace li {

template <typename C>
struct async_yield
{
  void operator()() { ctx.write(nullptr, 0); }
  void listen_to_fd(int fd) { ctx.listen_to_new_fd(fd); }
  C& ctx;
};

struct http_request {

  http_request(http_async_impl::http_ctx& http_ctx) : http_ctx(http_ctx), yield{http_ctx} {}
  
  inline std::string_view header(const char* k) const;
  inline std::string_view cookie(const char* k) const;

  // With list of parameters: s::id = int(), s::name = string(), ...
  template <typename S, typename V, typename... T>
  auto url_parameters(assign_exp<S, V> e, T... tail) const;
  template <typename S, typename V, typename... T>
  auto get_parameters(assign_exp<S, V> e, T... tail) const;
  template <typename S, typename V, typename... T>
  auto post_parameters(assign_exp<S, V> e, T... tail) const;

  // Const wrapper.
  template <typename O> auto url_parameters(const O& res) const;
  template <typename O> auto get_parameters(const O& res) const;
  template <typename O> auto post_parameters(const O& res) const;

  // With a metamap.
  template <typename O> auto url_parameters(O& res) const;
  template <typename O> auto get_parameters(O& res) const;
  template <typename O> auto post_parameters(O& res) const;

  http_async_impl::http_ctx& http_ctx;
  async_yield<http_async_impl::http_ctx> yield;
  std::string url_spec;
};

struct url_parser_info_node {
  int slash_pos;
  bool is_path;
};
using url_parser_info = std::unordered_map<std::string, url_parser_info_node>;

auto make_url_parser_info(const std::string_view url) {

  url_parser_info info;

  auto check_pattern = [](const char* s, char a) { return *s == a and *(s + 1) == a; };

  int slash_pos = -1;
  for (int i = 0; i < int(url.size()); i++) {
    if (url[i] == '/')
      slash_pos++;
    // param must start with {{
    if (check_pattern(url.data() + i, '{')) {
      const char* param_name_start = url.data() + i + 2;
      const char* param_name_end = param_name_start;
      // param must end with }}
      while (!check_pattern(param_name_end, '}'))
        param_name_end++;

      if (param_name_end != param_name_start and check_pattern(param_name_end, '}')) {
        int size = param_name_end - param_name_start;
        bool is_path = false;
        if (size > 3 and param_name_end[-1] == '.' and param_name_end[-2] == '.' and
            param_name_end[-3] == '.') {
          is_path = true;
          param_name_end -= 3;
        }
        std::string_view param_name(param_name_start, param_name_end - param_name_start);
        info.emplace(param_name, url_parser_info_node{slash_pos, is_path});
      }
    }
  }
  return info;
}

template <typename O>
auto parse_url_parameters(const url_parser_info& fmt, const std::string_view url, O& obj) {
  // get the indexes of the slashes in url.
  std::vector<int> slashes;
  for (int i = 0; i < int(url.size()); i++) {
    if (url[i] == '/')
      slashes.push_back(i);
  }

  // For each field in O...
  //  find the location of the field in the url thanks to fmt.
  //  get it.
  map(obj, [&](auto k, auto v) {
    const char* symbol_str = symbol_string(k);
    auto it = fmt.find(symbol_str);
    if (it == fmt.end()) {
      throw std::runtime_error(std::string("Parameter ") + symbol_str + " not found in url " +
                               url.data());
    } else {
      // Location of the parameter in the url.
      int param_slash = it->second.slash_pos; // index of slash before param.
      if (param_slash >= int(slashes.size()))
        throw http_error::bad_request("Missing url parameter ", symbol_str);

      int param_start = slashes[param_slash] + 1;
      if (it->second.is_path) {
        if constexpr (std::is_same<std::decay_t<decltype(obj[k])>, std::string>::value or
                      std::is_same<std::decay_t<decltype(obj[k])>, std::string_view>::value) {
          obj[k] = std::string_view(url.data() + param_start - 1,
                                    url.size() - param_start + 1); // -1 to include the first /.
        } else {
          throw std::runtime_error(
              "{{path...}} parameters only accept std::string or std::string_view types.");
        }

      } else {
        int param_end = param_start;
        while (int(url.size()) > (param_end) and url[param_end] != '/')
          param_end++;

        std::string_view content(url.data() + param_start, param_end - param_start);
        try {
          if constexpr(std::is_same<std::remove_reference_t<decltype(v)>, std::string>::value or
                      std::is_same<std::remove_reference_t<decltype(v)>, std::string_view>::value)
            obj[k] = content;
          else
            obj[k] = boost::lexical_cast<decltype(v)>(content);
        } catch (const std::bad_cast& e) {
          throw http_error::bad_request("Cannot decode url parameter ", li::symbol_string(k), " : ",
                                        e.what());
        }
      }
    }
  });
  return obj;
}

inline std::string_view http_request::header(const char* k) const {
  return http_ctx.header(k);
}

inline std::string_view http_request::cookie(const char* k) const {
  return http_ctx.cookie(k);
  //FIXME return MHD_lookup_connection_value(mhd_connection, MHD_COOKIE_KIND, k);
}

template <typename S, typename V, typename... T>
auto http_request::url_parameters(assign_exp<S, V> e, T... tail) const {
  return url_parameters(mmm(e, tail...));
}

template <typename S, typename V, typename... T>
auto http_request::get_parameters(assign_exp<S, V> e, T... tail) const {
  return get_parameters(mmm(e, tail...));
}

template <typename S, typename V, typename... T>
auto http_request::post_parameters(assign_exp<S, V> e, T... tail) const {
  auto o = mmm(e, tail...);
  return post_parameters(o);
}

template <typename O> auto http_request::url_parameters(const O& res) const {
  O r;
  return url_parameters(r);
}

template <typename O> auto http_request::get_parameters(const O& res) const {
  O r;
  return get_parameters(r);
}
template <typename O> auto http_request::post_parameters(const O& res) const {
  O r;
  return post_parameters(r);
}

template <typename O> auto http_request::url_parameters(O& res) const {
  auto info = make_url_parser_info(url_spec);
  return parse_url_parameters(info, http_ctx.url(), res);
}

template <typename O> auto http_request::get_parameters(O& res) const {

  try {
    url_decode(http_ctx.get_parameters_string(), res);
  }
   catch (const std::runtime_error& e)
  {
    throw http_error::bad_request("Error while decoding the GET parameter: ", e.what());
  }

  return res;
}

template <typename O> auto http_request::post_parameters(O& res) const {
  try {
    std::string_view encoding = this->header("Content-Type");
    if (!encoding.data())
      throw http_error::bad_request(
          std::string("Content-Type is required to decode the POST parameters"));

    std::string_view body = http_ctx.read_whole_body();
    if (encoding == std::string_view("application/x-www-form-urlencoded"))
      url_decode(url_unescape(body), res);
    else if (encoding == std::string_view("application/json"))
      json_decode(body, res);
  } catch (std::exception e) {
    throw http_error::bad_request("Error while decoding the POST parameters: ", e.what());
  }

  return res;
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_REQUEST

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_RESPONSE
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_RESPONSE


//#include <stdlib.h>

#if defined(_MSC_VER)
#endif

//#include <sys/stat.h>

namespace li {
using namespace li;

struct http_response {
  inline http_response(http_async_impl::http_ctx& ctx) : http_ctx(ctx) {}

  inline void set_header(std::string_view k, std::string_view v) { http_ctx.set_header(k, v); }
  inline void set_cookie(std::string_view k, std::string_view v) { http_ctx.set_cookie(k, v); }

  template <typename O>
  inline void write_json(O&& obj) {     
    http_ctx.set_header("Content-Type", "application/json");
    http_ctx.respond_json(std::forward<O>(obj));
  }
  template <typename A, typename B, typename... O>
  void write_json(assign_exp<A, B>&& w1, O&&... ws) {
    write_json(mmm(std::forward<assign_exp<A, B>>(w1), std::forward<O>(ws)...));
  }

  inline void write() { http_ctx.respond(body); }

  void set_status(int s) { http_ctx.set_status(s); }

  template <typename A1, typename... A> inline void write(A1&& a1, A&&... a) {
    body += boost::lexical_cast<std::string>(std::forward<A1>(a1));
    write(std::forward<A>(a)...);
  }
  
  template <typename A1, typename... A> inline void write(std::string_view a1) 
  {
    http_ctx.respond(a1); 
  }

  inline void write_file(const std::string path) {
    http_ctx.send_static_file(path.c_str());
  }

  http_async_impl::http_ctx& http_ctx;
  std::string body;
};

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_RESPONSE


namespace li {

template <typename... O> auto http_serve(api<http_request, http_response> api, int port, O... opts) {

  auto options = mmm(opts...);

//int http_serve(int port, int nthreads, F handler)
  auto handler = [api] (http_async_impl::http_ctx& ctx) {
    http_request rq{ctx};
    http_response resp(ctx);
    try {
      api.call(ctx.method(), ctx.url(), rq, resp);
    } catch (const http_error& e) {
      ctx.set_status(e.status());
      ctx.respond(e.what());
    } catch (const std::runtime_error& e) {
      std::cerr << "INTERNAL SERVER ERROR: "<< e.what() << std::endl;
      ctx.set_status(500);
      ctx.respond("Internal server error.");
    }
    ctx.respond_if_needed();
  };

  auto date_thread = std::make_shared<std::thread>([&] () {
      char a1[100];
      char a2[100];
      memset(a1, 0, sizeof(a1));
      memset(a2, 0, sizeof(a2));
      char* date_buf_tmp1 = a1;
      char* date_buf_tmp2 = a2;
      while (!moustique_exit_request)
      {
        time_t t = time(NULL);
        const tm& tm = *gmtime(&t);
        int size = strftime(date_buf_tmp1, sizeof(a1), "%a, %d %b %Y %T GMT", &tm);
        http_async_impl::date_buf = date_buf_tmp1;
        http_async_impl::date_buf_size = size;
        std::swap(date_buf_tmp1, date_buf_tmp2);
        usleep(1e6);
      }
    });

  auto server_thread = std::make_shared<std::thread>([=] () {
    std::cout << "Starting lithium::http_backend on port " << port << std::endl;
    moustique_listen(port, SOCK_STREAM, 4, http_async_impl::make_http_processor(std::move(handler)));
    date_thread->join();
  });

  if constexpr (has_key<decltype(options), s::non_blocking_t>())
  {
    usleep(0.1e6);
    date_thread->detach();
    server_thread->detach();
    //return mmm(s::server_thread = server_thread, s::date_thread = date_thread);
  }
  else
    server_thread->join();

}
}
#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_HTTP_LISTEN2


namespace li {
using http_api = api<http_request, http_response>;
}

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_HASHMAP_HTTP_SESSION
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_HASHMAP_HTTP_SESSION

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_RANDOM_COOKIE
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_RANDOM_COOKIE


namespace li {

std::string generate_secret_tracking_id() {
  std::ostringstream os;
  std::random_device rd;
  os << std::hex << rd() << rd() << rd() << rd();
  return os.str();
}

inline std::string random_cookie(http_request& request, http_response& response,
                                 const char* key = "silicon_token") {
  std::string token;
  std::string_view token_ = request.cookie(key);
  if (!token_.data()) {
    token = generate_secret_tracking_id();
    response.set_cookie(key, token);
  } else {
    std::cout << "got token " << token_ << std::endl;
    token = token_;
  }

  return token;
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_RANDOM_COOKIE


namespace li {

template <typename O> struct connected_hashmap_http_session {

  connected_hashmap_http_session(std::string session_id_, O* values_,
                                 std::unordered_map<std::string, O>& sessions_, std::mutex& mutex_)
      : session_id_(session_id_), values_(values_), sessions_(sessions_), mutex_(mutex_) {}

  // Store fiels into the session
  template <typename... F> auto store(F... fields) {
    map(mmm(fields...), [this](auto k, auto v) { values()[k] = v; });
  }

  // Access values of the session.
  O& values() {
    if (!values_)
      throw http_error::internal_server_error("Cannot access session values after logout.");
    return *values_;
  }

  // Delete the session from the database.
  void logout() {
    std::lock_guard<std::mutex> guard(mutex_);
    sessions_.erase(session_id_);
  }

private:
  std::string session_id_;
  O* values_;
  std::unordered_map<std::string, O>& sessions_;
  std::mutex& mutex_;
};

template <typename... F> struct hashmap_http_session {

  typedef decltype(mmm(std::declval<F>()...)) session_values_type;
  inline hashmap_http_session(std::string cookie_name, F... fields)
      : cookie_name_(cookie_name), default_values_(mmm(s::session_id = std::string(), fields...)) {}

  inline auto connect(http_request& request, http_response& response) {
    std::string session_id = random_cookie(request, response, cookie_name_.c_str());
    auto it = sessions_.try_emplace(session_id, default_values_).first;
    return connected_hashmap_http_session<session_values_type>{session_id, &it->second, sessions_,
                                                               sessions_mutex_};
  }

  std::string cookie_name_;
  session_values_type default_values_;
  std::unordered_map<std::string, session_values_type> sessions_;
  std::mutex sessions_mutex_;
};

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_HASHMAP_HTTP_SESSION

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_HTTP_AUTHENTICATION
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_HTTP_AUTHENTICATION


#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_SQL_HTTP_SESSION
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_SQL_HTTP_SESSION


namespace li {

template <typename ORM> struct connected_sql_http_session {

  // Construct the session.
  // Retrive the cookie
  // Retrieve it from the database.
  // Insert it if it does not exists.
  connected_sql_http_session(typename ORM::object_type& defaults, ORM orm,
                             const std::string& session_id)
      : loaded_(false), session_id_(session_id), orm_(orm), values_(defaults) {}

  // Store fiels into the session
  template <typename... F> auto store(F... fields) {
    map(mmm(fields...), [this](auto k, auto v) { values_[k] = v; });
    if (!orm_.exists(s::session_id = session_id_))
      orm_.insert(s::session_id = session_id_, fields...);
    else
      orm_.update(s::session_id = session_id_, fields...);
  }

  // Access values of the session.
  const auto values() {
    load();
    return values_;
  }

  // Delete the session from the database.
  void logout() { orm_.remove(s::session_id = session_id_); }

private:
  auto load() {
    if (loaded_)
      return;
    if (auto new_values_ = orm_.find_one(s::session_id = session_id_))
      values_ = *new_values_;
    loaded_ = true;
  }

  bool loaded_;
  std::string session_id_;
  ORM orm_;
  typename ORM::object_type values_;
};

template <typename DB, typename... F>
decltype(auto) create_session_orm(DB& db, std::string table_name, F... fields) {
  return sql_orm_schema<DB>(db, table_name)
      .fields(s::session_id(s::read_only, s::primary_key) = sql_varchar<32>(), fields...);
}

template <typename DB, typename... F> struct sql_http_session {

  sql_http_session(DB& db, std::string table_name, std::string cookie_name, F... fields)
      : cookie_name_(cookie_name),
        default_values_(mmm(s::session_id = sql_varchar<32>(), fields...)),
        session_table_(create_session_orm(db, table_name, fields...)) {}

  auto connect(http_request& request, http_response& response) {
    return connected_sql_http_session(default_values_, session_table_.connect(request.yield),
                                      random_cookie(request, response, cookie_name_.c_str()));
  }

  auto orm() { return session_table_; }

  std::string cookie_name_;
  std::decay_t<decltype(mmm(s::session_id = sql_varchar<32>(), std::declval<F>()...))>
      default_values_;
  std::decay_t<decltype(
      create_session_orm(std::declval<DB&>(), std::string(), std::declval<F>()...))>
      session_table_;
};

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_SQL_HTTP_SESSION


namespace li {

template <typename S, typename U, typename L, typename P, typename... CB>
struct http_authentication {
  http_authentication(S& session, U& users, L login_field, P password_field, CB... callbacks)
      : sessions_(session), users_(users), login_field_(login_field),
        password_field_(password_field), callbacks_(mmm(callbacks...)) {

    auto allowed_callbacks = mmm(s::hash_password, s::create_secret_key);

    static_assert(metamap_size<decltype(substract(callbacks_, allowed_callbacks))>() == 0,
                  "The only supported callbacks for http_authentication are: s::hash_password, "
                  "s::create_secret_key");
  }

  template <typename SS, typename... A> void call_callback(SS s, A&&... args) {
    if constexpr (has_key<decltype(callbacks_)>(s))
      return callbacks_[s](std::forward<A>(args)...);
  }

  bool login(http_request& req, http_response& resp) {
    auto lp = req.post_parameters(login_field_ = users_.all_fields()[login_field_],
                                  password_field_ = users_.all_fields()[password_field_]);

    if constexpr (has_key<decltype(callbacks_)>(s::hash_password))
      lp[password_field_] = callbacks_[s::hash_password](lp[login_field_], lp[password_field_]);

    if (auto user = users_.connect(req.yield).find_one(lp)) {
      sessions_.connect(req, resp).store(s::user_id = user->id);
      return true;
    } else
      return false;
  }

  auto current_user(http_request& req, http_response& resp) {
    auto sess = sessions_.connect(req, resp);
    if (sess.values().user_id != -1)
      return users_.connect().find_one(s::id = sess.values().user_id);
    else
      return decltype(users_.connect(req.yield).find_one(s::id = sess.values().user_id)){};
  }

  void logout(http_request& req, http_response& resp) { sessions_.connect(req, resp).logout(); }

  bool signup(http_request& req, http_response& resp) {
    auto new_user = req.post_parameters(users_.all_fields_except_computed());
    auto users = users_.connect(req.yield);

    if (users.exists(login_field_ = new_user[login_field_]))
      return false;
    else {
      if constexpr (has_key<decltype(callbacks_)>(s::update_secret_key))
        callbacks_[s::update_secret_key](new_user[login_field_], new_user[password_field_]);
      if constexpr (has_key<decltype(callbacks_)>(s::hash_password))
        new_user[password_field_] =
            callbacks_[s::hash_password](new_user[login_field_], new_user[password_field_]);
      users.insert(new_user);
      return true;
    }
  }

  S& sessions_;
  U& users_;
  L login_field_;
  P password_field_;
  decltype(mmm(std::declval<CB>()...)) callbacks_;
};

template <typename... A> http_api http_authentication_api(http_authentication<A...>& auth) {

  http_api api;

  api.post("/login") = [&](http_request& request, http_response& response) {
    if (!auth.login(request, response))
      throw http_error::unauthorized("Bad login.");
  };

  api.get("/logout") = [&](http_request& request, http_response& response) {
    auth.logout(request, response);
  };

  api.get("/signup") = [&](http_request& request, http_response& response) {
    if (!auth.signup(request, response))
      throw http_error::bad_request("User already exists.");
  };

  return api;
}

// Disable this for now. (No time to install nettle on windows.)
// #include <nettle/sha3.h>
// inline std::string hash_sha3_512(const std::string& str)
// {
//   struct sha3_512_ctx ctx;
//   sha3_512_init(&ctx);
//   sha3_512_update(&ctx, str.size(), (const uint8_t*) str.data());
//   uint8_t h[SHA3_512_DIGEST_SIZE];
//   sha3_512_digest(&ctx, sizeof(h), h);
//   return std::string((const char*)h, sizeof(h));
// }

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_HTTP_AUTHENTICATION

//#include <li/http_backend/mhd.hh>
#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_SERVE_DIRECTORY
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_SERVE_DIRECTORY


namespace li {

auto serve_file(const std::string& root, std::string_view path, http_response& response) {
  std::string base_path = root;
  if (!base_path.empty() && base_path[base_path.size() - 1] != '/') {
    base_path.push_back('/');
  }

  static char dot = '.', slash = '/';

  // remove first slahs if needed.
  // std::string path(r->url.substr(prefix.string().size(), r->url.size() -
  // prefix.string().size()));
  size_t len = path.size();
  if (!path.empty() && path[0] == slash) {
    path = std::string_view(path.data() + 1, path.size() - 1);//erase(0, 1);
  }

  if (path.empty()) {
    throw http_error::bad_request("No file path given, directory listing not supported.");
  }

  if (len == 1 && path[0] == dot) {
    throw http_error::bad_request("Invalid URI ", path);
  } else if (len == 2 && path[0] == dot && path[1] == dot) {
    throw http_error::bad_request("Invalid URI ", path);
  } else {
    char prev0 = slash, prev1 = slash;
    for (size_t i = 0; i < len; ++i) {
      if (prev0 == dot && prev1 == dot && path[i] == slash) {
        throw http_error::bad_request("Unsupported URI, ../ is not allowed in the URI");
      }
      prev0 = prev1;
      prev1 = path[i];
    }
  }

  response.write_file(base_path + std::string(path));
};

inline auto serve_directory(std::string root) {
  http_api api;

  api.get("/{{path...}}") = [root](http_request& request, http_response& response) {
    auto path = request.url_parameters(s::path = std::string_view()).path;
    return serve_file(root, path, response);
  };
  return api;
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_SERVE_DIRECTORY

#ifndef LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_SQL_CRUD_API
#define LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_SQL_CRUD_API


namespace li {

template <typename A, typename B, typename C>
auto sql_crud_api(sql_orm_schema<A, B, C>& orm_schema) {

  http_api api;

  api.post("/find_by_id") = [&](http_request& request, http_response& response) {
    auto params = request.post_parameters(s::id = int());
    if (auto obj = orm_schema.connect(request.yield).find_one(s::id = params.id, request, response))
      response.write_json(obj);
    else
      throw http_error::not_found(orm_schema.table_name(), " with id ", params.id,
                                  " does not exist.");
  };

  api.post("/create") = [&](http_request& request, http_response& response) {
    auto insert_fields = orm_schema.all_fields_except_computed();
    auto obj = request.post_parameters(insert_fields);
    long long int id = orm_schema.connect(request.yield).insert(obj, request, response);
    response.write_json(s::id = id);
  };

  api.post("/update") = [&](http_request& request, http_response& response) {
    auto obj = request.post_parameters(orm_schema.all_fields());
    orm_schema.connect(request.yield).update(obj, request, response);
  };

  api.post("/remove") = [&](http_request& request, http_response& response) {
    auto obj = request.post_parameters(orm_schema.primary_key());
    orm_schema.connect(request.yield).remove(obj, request, response);
  };

  return api;
}

} // namespace li

#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_SQL_CRUD_API


#endif // LITHIUM_SINGLE_HEADER_GUARD_LI_HTTP_BACKEND_HTTP_BACKEND

