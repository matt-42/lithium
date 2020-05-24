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
  T res = boost::lexical_cast<T>(row[0]);
  mysql_wrapper_.mysql_free_result(connection_status_, result_);
  return res;
}

} // namespace li

#include <li/sql/mysql_result.hpp>
