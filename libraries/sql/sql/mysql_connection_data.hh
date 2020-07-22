#pragma once

#include <unordered_map>
#include <mysql.h>

#include <li/sql/type_hashmap.hh>

namespace li
{

/**
 * @brief Data of a connection.
 *
 */
struct mysql_connection_data {

  ~mysql_connection_data() { mysql_close(connection_); }

  MYSQL* connection_;
  std::unordered_map<std::string, std::shared_ptr<mysql_statement_data>> statements_;
  type_hashmap<std::shared_ptr<mysql_statement_data>> statements_hashmap_;
  int error_ = 0;
};

}