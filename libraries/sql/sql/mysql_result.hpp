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
#include <thread>
#include <tuple>
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
    result_ = mysql_use_result(connection_->connection_);
  current_row_ = mysql_wrapper_.mysql_fetch_row(connection_->error_, result_);
  current_row_num_fields_ = mysql_num_fields(result_);
  if (!current_row_ || current_row_num_fields_ == 0) {
    end_of_result_ = true;
    return;
  }

  current_row_lengths_ = mysql_fetch_lengths(result_);
}

template <typename B> template <typename T> bool mysql_result<B>::read(T&& output) {

  next_row();

  if (end_of_result_)
    return false;

  if constexpr (is_tuple<T>::value) { // Tuple

    if (std::tuple_size_v<std::decay_t<T>> != current_row_num_fields_)
      throw std::runtime_error(std::string("The request number of field (") +
                               boost::lexical_cast<std::string>(current_row_num_fields_) +
                               ") does not match the size of the tuple (" +
                               boost::lexical_cast<std::string>(std::tuple_size_v<std::decay_t<T>>) + ")");
    int i = 0;
    li::tuple_map(std::forward<T>(output), [&](auto& v) {
      // std::cout << "read " << std::string_view(current_row_[i], current_row_lengths_[i]) << std::endl;
      v = boost::lexical_cast<std::decay_t<decltype(v)>>(
          std::string_view(current_row_[i], current_row_lengths_[i]));
      i++;
    });

  } else { // Metamap.

    if (li::metamap_size(output) != current_row_num_fields_)
      throw std::runtime_error(
          "The request number of field does not match the size of the metamap object.");
    int i = 0;
    li::map(std::forward<T>(output), [&](auto k, auto& v) {
      v = boost::lexical_cast<decltype(v)>(
          std::string_view(current_row_[i], current_row_lengths_[i]));
      i++;
    });
  }

  return true;
}

template <typename B> long long int mysql_result<B>::affected_rows() {
  return mysql_affected_rows(connection_->connection_);
}

template <typename B> long long int mysql_result<B>::last_insert_id() {
  return mysql_insert_id(connection_->connection_);
}

} // namespace li

#include <li/sql/mysql_result.hpp>
