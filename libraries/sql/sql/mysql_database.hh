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

#include <li/sql/mysql_connection.hh>

namespace li {

struct mysql_database_impl {

  typedef mysql_tag db_tag;
  typedef connection_type mysql_connection;
  typedef connection_data_type mysql_connection_data;

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
  template <typename... O> inline mysql_database(O... opts);
  
  inline ~mysql_database();

  template <typename Y> inline mysql_connection<mysql_functions_non_blocking<Y>> new_connection(Y& fiber);
  inline int get_socket(std::shared_ptr<mysql_connection_data> data);

  std::string host_, user_, passwd_, database_;
  unsigned int port_;
  std::string character_set_;
};

typedef sql_database<mysql_database_impl> mysql_database;

} // namespace li

#include <li/sql/mysql_database.hpp>
