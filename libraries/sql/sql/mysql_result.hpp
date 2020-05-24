#pragma once

#include <atomic>
#include <boost/lexical_cast.hpp>
#include <cassert>
#include <cstring>
#include <deque>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <mysql.h>
#include <optional>
#include <sstream>
#include <sys/epoll.h>
#include <thread>
#include <unordered_map>

#include <li/callable_traits/callable_traits.hh>
#include <li/metamap/metamap.hh>
#include <li/sql/mysql_async_wrapper.hh>
#include <li/sql/mysql_statement.hh>
#include <li/sql/mysql_statement_result.hh>
#include <li/sql/sql_common.hh>
#include <li/sql/symbols.hh>
#include <li/sql/type_hashmap.hh>

namespace li {

template <typename B> template <typename T> T mysql_result<B>::read() {
  result_ = mysql_use_result(con_);
  MYSQL_ROW row = mysql_wrapper_.mysql_fetch_row(connection_status_, result_);

  if (!row || mysql_num_fields(result_) == 0)
    throw std::runtime_error("Request did not return any data.");

  T res = boost::lexical_cast<T>(row[0]);

  // Flush remaining rows if needed.
  while (row = mysql_wrapper_.mysql_fetch_row(connection_status_, result_))
    ;

  mysql_wrapper_.mysql_free_result(connection_status_, result_);
  return res;
}

template <typename B> long long int mysql_result<B>::affected_rows() {
  return mysql_affected_rows(con_);
}

template <typename B> template <typename T> void mysql_result<B>::read(std::optional<T>& o) {
  result_ = mysql_use_result(con_);
  MYSQL_ROW row = mysql_wrapper_.mysql_fetch_row(connection_status_, result_);

  if (!row || mysql_num_fields(result_) == 0)
  {
    o = std::optional<T>();
    return;
  }
  T res = boost::lexical_cast<T>(row[0]);

  // Flush remaining rows if needed.
  while (row = mysql_wrapper_.mysql_fetch_row(connection_status_, result_))
    ;

  mysql_wrapper_.mysql_free_result(connection_status_, result_);
  o = std::optional<T>(res);
}

template <typename B> template <typename T> std::optional<T> mysql_result<B>::read_optional() {
  std::optional<T> t;
  this->read(t);
  return t;
}

// template <typename T> struct unconstref_tuple_elements {};
// template <typename... T> struct unconstref_tuple_elements<std::tuple<T...>> {
//   typedef std::tuple<std::remove_const_t<std::remove_reference_t<T>>...> ret;
// };

template <typename B> template <typename F> void mysql_result<B>::map(F f) {

  typedef typename unconstref_tuple_elements<callable_arguments_tuple_t<F>>::ret tp;
  typedef std::remove_const_t<std::remove_reference_t<std::tuple_element_t<0, tp>>> T;

  auto map_function_argument = []() {
    if constexpr (li::is_metamap<T>::value)
      return T{}; // Simgle metamap object.
    else
      return tp{}; // A tuple of arguments.
  }();

  result_ = mysql_use_result(con_);

  MYSQL_ROW row;
  int num_fields = mysql_num_fields(result_);
  while ((row = mysql_fetch_row(result_))) {

    unsigned long* lengths;
    lengths = mysql_fetch_lengths(result_);
    // for (i = 0; i < num_fields; i++) {
    //   printf("[%.*s] ", (int)lengths[i], row[i] ? row[i] : "NULL");
    // }
    // printf("\n");

    if constexpr (li::is_metamap<T>::value) {
      if (li::metamap_size<T>() != num_fields)
        throw std::runtime_error(
            "The request number of field does not match the size of the metamap object.");
      int i = 0;
      li::map(map_function_argument, [&](auto k, auto& v) {
        v = boost::lexical_cast<decltype(v)>(std::string_view(row[i], lengths[i]));
        i++;
      });
      f(map_function_argument);
    } else {
      if (std::tuple_size<tp>() != num_fields)
        throw std::runtime_error("The request number of field does not match the number of "
                                 "arguments of the mapped function.");
      int i = 0;
      li::tuple_map(map_function_argument, [&](auto& v) {
        v = boost::lexical_cast<std::decay_t<decltype(v)>>(std::string_view(row[i], lengths[i]));
        i++;
      });
      std::apply(f, map_function_argument);
    }
  }

  mysql_wrapper_.mysql_free_result(connection_status_, result_);
}

// template <typename B> void mysql_result<B>::wait() {}

template <typename B> long long int mysql_result<B>::last_insert_id() {
  return mysql_insert_id(con_);
}

template <typename B> bool mysql_result<B>::empty() {
  return false; // mysql_wrapper_.mysql_stmt_fetch(connection_status_, data_.stmt_) ==
                // MYSQL_NO_DATA;
}
} // namespace li

#include <li/sql/mysql_result.hpp>
