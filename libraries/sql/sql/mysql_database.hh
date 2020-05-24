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

struct mysql_database : std::enable_shared_from_this<mysql_database> {

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

  /**
   * @brief Provide a new mysql non blocking connection. The connection provides RAII: it will be
   * placed back in the available connection pool whenver its constructor is called.
   *
   * @param fiber the fiber object providing the 3 non blocking logic methods:
   *
   *    - void epoll_add(int fd, int flags); // Make the current epoll fiber wakeup on
   *                                            file descriptor fd 
   *    - void epoll_mod(int fd, int flags); // Modify the epoll flags on file
   *                                            descriptor fd 
   *    - void yield() // Yield the current epoll fiber.
   *
   * @return mysql_connection<mysql_functions_non_blocking<Y>>
   */
  template <typename Y> inline mysql_connection<mysql_functions_non_blocking<Y>> connect(Y& fiber);

  /**
   * @brief Provide a new mysql blocking connection. The connection provides RAII: it will be
   * placed back in the available connection pool whenver its constructor is called.
   *
   * @return the connection.
   */
  inline mysql_connection<mysql_functions_blocking> connect();

  std::mutex mutex_;
  std::string host_, user_, passwd_, database_;
  unsigned int port_;
  std::deque<MYSQL*> _;
  std::string character_set_;
};

} // namespace li

#include <li/sql/mysql_database.hpp>
