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

template <typename B> void mysql_result<B>::next_row() {

  if (!result_)
    result_ = mysql_use_result(con_);
  curent_row_ = mysql_wrapper_.mysql_fetch_row(connection_status_, result_);

  if (!row || mysql_num_fields(result_) == 0) {
    end_of_result_ = true;
    return;
  }

  current_row_lengths_ = mysql_fetch_lengths(result_);
}

template <typename B> template <typename T> bool mysql_result<B>::read(T&& output) {

  if (end_of_result_)
    return false;

  if constexpr (is_tuple<T>::value) { // Tuple

    if (std::tuple_size<T1>() != num_fields)
      throw std::runtime_error("The request number of field does not match the size of "
                               "the tuple.");
    int i = 0;
    li::tuple_map(std::forward<T>(output), [&](auto& v) {
      v = boost::lexical_cast<std::decay_t<decltype(v)>>(
          std::string_view(row[i], current_row_lengths_[i]));
      i++;
    });

  } else { // Metamap.

    static_assert(sizeof...(T) != 0);

    if (sizeof...(T) != mysql_num_fields(result_))
      throw std::runtime_error(
          "The request number of field does not match the size of the metamap object.");
    int i = 0;
    li::map(std::forward<T>(output), [&](auto k, auto& v) {
      v = boost::lexical_cast<decltype(v)>(std::string_view(row[i], current_row_lengths_[i]));
      i++;
    });
  }

  next_row();
  return true;
}

template <typename B> long long int mysql_result<B>::affected_rows() {
  return mysql_affected_rows(con_);
}

template <typename B> long long int mysql_result<B>::last_insert_id() {
  return mysql_insert_id(con_);
}

} // namespace li

#include <li/sql/mysql_result.hpp>
