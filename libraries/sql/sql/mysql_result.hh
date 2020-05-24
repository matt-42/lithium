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

/**
 * @brief Store a access to the result of a sql query (non prepared).
 *
 * @tparam B must be mysql_functions_blocking or mysql_functions_non_blocking
 */
template <typename B> struct mysql_result {

  B& mysql_wrapper_; // blocking or non blockin mysql functions wrapper.
  MYSQL* con_; // Mysql connection
  std::shared_ptr<int> connection_status_; // Status of the connection
  MYSQL_RES* result_ = nullptr; // Mysql result.

  /**
   * @brief Read a single valued request.
   *
   * @tparam T type of the value to read.
   * @return T
   */
  template <typename T> T read();

  void wait();
};

} // namespace li

#include <li/sql/mysql_result.hpp>
