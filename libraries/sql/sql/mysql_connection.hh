#pragma once

#include <deque>
#include <memory>
#include <li/sql/type_hashmap.hh>
#include <li/sql/mysql_result.hh>

namespace li {

int max_mysql_connections_per_thread = 200;

// Forward ref.
struct mysql_connection_data;

// Pools of connection
thread_local std::deque<std::shared_ptr<mysql_connection_data>> mysql_connection_pool;
thread_local std::deque<std::shared_ptr<mysql_connection_data>> mysql_connection_async_pool;
thread_local int total_number_of_mysql_connections = 0;

struct mysql_tag {};

template <typename B> // must be mysql_functions_blocking or mysql_functions_non_blocking
struct mysql_connection {

  typedef mysql_tag db_tag;

  /**
   * @brief Construct a new mysql connection object.
   *
   * @param mysql_wrapper the [non]blocking mysql wrapper.
   * @param data the connection data.
   */
  inline mysql_connection(B mysql_wrapper, std::shared_ptr<li::mysql_connection_data> data);

  /**
   * @brief Last inserted row id.
   *
   * @return long long int the row id.
   */
  long long int last_insert_rowid();

  /**
   * @brief Execute a SQL request.
   *
   * @param rq the request string
   * @return mysql_result<B> the result.
   */
  mysql_result<B> operator()(const std::string& rq);

  /**
   * @brief Build a sql prepared statement.
   *
   * @param rq the request string
   * @return mysql_statement<B> the statement.
   */
  mysql_statement<B> prepare(const std::string& rq);

  /**
   * @brief Build or retrieve a sql statement from the connection cache.
   * Will regenerate the statement if one of the @param keys changed.
   *
   * @param f the function that generate the statement.
   * @param keys the keys.
   * @return mysql_statement<B> the statement.
   */
  template <typename F, typename... K> mysql_statement<B> cached_statement(F f, K... keys);

  template <typename T> std::string type_to_string(const T& t) { return cpptype_to_mysql_type(t); }

  B mysql_wrapper_;
  std::shared_ptr<mysql_connection_data> data_;
  std::shared_ptr<int> connection_status_;
};

/**
 * @brief Data of a connection.
 *
 */
struct mysql_connection_data {

  ~mysql_connection_data();

  MYSQL* connection_;
  std::unordered_map<std::string, std::shared_ptr<mysql_statement_data>> statements_;
  type_hashmap<std::shared_ptr<mysql_statement_data>> statements_hashmap_;
};

} // namespace li

#include <li/sql/mysql_connection.hpp>
