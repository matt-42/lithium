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
#include <unordered_map>

#include <li/callable_traits/callable_traits.hh>
#include <li/metamap/metamap.hh>
#include <li/sql/mysql_async_wrapper.hh>
#include <li/sql/mysql_statement.hh>
#include <li/sql/mysql_statement_result.hh>
#include <li/sql/sql_common.hh>
#include <li/sql/symbols.hh>
#include <li/sql/type_hashmap.hh>

#include <li/sql/mysql_connection.hh>
#include <li/sql/sql_database.hh>

namespace li {

struct mysql_database_impl {

  typedef mysql_tag db_tag;
  typedef mysql_connection_data connection_data_type;

  /**
   * @brief Construct a new mysql database object
   *
   *
   * @tparam O
   * @param opts Available options are:
   *                 - s::host = "hostname or ip"
   *                 - s::database = "database name"
   *                 - s::user = "username"
   *                 - s::password = "user passord"
   *                 - s::charset = "character set" default: utf8
   *
   */
  template <typename... O> inline mysql_database_impl(O... opts);

  inline ~mysql_database_impl();

  template <typename Y> inline mysql_connection_data* new_connection(Y& fiber);
  inline int get_socket(const std::shared_ptr<mysql_connection_data>& data);

  template <typename Y>
  inline auto scoped_connection(Y& fiber, std::shared_ptr<mysql_connection_data>& data);

  std::string host_, user_, passwd_, database_;
  unsigned int port_;
  std::string character_set_;
};

typedef sql_database<mysql_database_impl> mysql_database;

} // namespace li

#include <li/sql/mysql_database.hpp>
