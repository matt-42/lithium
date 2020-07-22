#pragma once

#include <mysql.h>

#include <li/callable_traits/callable_traits.hh>
#include <li/metamap/metamap.hh>
#include <li/sql/sql_common.hh>
#include <li/sql/symbols.hh>

#include <li/sql/internal/mysql_bind.hh>
#include <li/sql/internal/mysql_statement_data.hh>
#include <li/sql/sql_result.hh>
#include <li/sql/mysql_async_wrapper.hh>
#include <li/sql/mysql_statement_result.hh>

namespace li {

/**
 * @brief Mysql prepared statement
 *
 * @tparam B the blocking or non blocking mode.
 */
template <typename B> struct mysql_statement {

  /**
   * @brief Execute the statement with argument.
   * Number of args must be equal to the number of placeholders in the request.
   *
   * @param args the arguments
   * @return mysql_statement_result<B> the result
   */
  template <typename... T> sql_result<mysql_statement_result<B>> operator()(T&&... args);

  B& mysql_wrapper_;
  mysql_statement_data& data_;
  std::shared_ptr<mysql_connection_data> connection_;
};

} // namespace li

#include <li/sql/mysql_statement.hpp>
