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

template <typename... O> inline mysql_database_impl::mysql_database_impl(O... opts) {

  auto options = mmm(opts...);
  static_assert(has_key(options, s::host), "open_mysql_connection requires the s::host argument");
  static_assert(has_key(options, s::database),
                "open_mysql_connection requires the s::databaser argument");
  static_assert(has_key(options, s::user), "open_mysql_connection requires the s::user argument");
  static_assert(has_key(options, s::password),
                "open_mysql_connection requires the s::password argument");

  host_ = options.host;
  database_ = options.database;
  user_ = options.user;
  passwd_ = options.password;
  port_ = get_or(options, s::port, 3306);
  character_set_ = get_or(options, s::charset, "utf8");

  if (mysql_library_init(0, NULL, NULL))
    throw std::runtime_error("Could not initialize MySQL library.");
  if (!mysql_thread_safe())
    throw std::runtime_error("Mysql is not compiled as thread safe.");
}

mysql_database_impl::~mysql_database_impl() { mysql_library_end(); }

inline int mysql_database_impl::get_socket(std::shared_ptr<mysql_connection_data> data) {
  return mysql_get_socket(data->connection_);
}

template <typename Y>
inline std::shared_ptr<mysql_connection_data> mysql_database_impl::new_connection(Y& fiber) {

  MYSQL* mysql;
  int mysql_fd = -1;
  int status;
  MYSQL* connection;

  mysql = new MYSQL;
  mysql_init(mysql);

  if constexpr (std::is_same_v<Y, active_yield>) { // Synchronous connection
    connection = mysql;
    connection = mysql_real_connect(connection, host_.c_str(), user_.c_str(), passwd_.c_str(),
                                    database_.c_str(), port_, NULL, 0);
    if (!connection)
      return nullptr;
  } else { // Async connection.
    mysql_options(mysql, MYSQL_OPT_NONBLOCK, 0);
    connection = nullptr;
    status = mysql_real_connect_start(&connection, mysql, host_.c_str(), user_.c_str(),
                                      passwd_.c_str(), database_.c_str(), port_, NULL, 0);

    // std::cout << "after: " << mysql_get_socket(mysql) << " " << status == MYSQL_ <<
    // std::endl;
    mysql_fd = mysql_get_socket(mysql);
    if (mysql_fd == -1) {
      // std::cout << "Invalid mysql connection bad mysql_get_socket " << status << " " << mysql
      // << std::endl;
      mysql_close(mysql);
      return nullptr;
    }

    if (status)
      fiber.epoll_add(mysql_fd, EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET);
    while (status)
      try {
        fiber.yield();
        status = mysql_real_connect_cont(&connection, mysql, status);
      } catch (typename Y::exception_type& e) {
        // Yield thrown a exception (probably because a closed connection).
        // std::cerr << "Warning: yield threw an exception while connecting to mysql: "
        //  << total_number_of_mysql_connections << std::endl;
        mysql_close(mysql);
        throw std::move(e);
      }
    if (!connection) {
      // Error in mysql_real_connect_cont
      return nullptr;
    }
  }

  mysql_set_character_set(mysql, character_set_.c_str());
  return std::shared_ptr<mysql_connection_data>(new mysql_connection_data{mysql});
}

template <typename Y, typename F>
inline auto mysql_database_impl::scoped_connection(Y& fiber,
                                                   std::shared_ptr<mysql_connection_data>& data,
                                                   F put_back_in_pool) {
  if constexpr (std::is_same_v<active_yield, Y>)
    return mysql_connection(mysql_functions_blocking{}, data, put_back_in_pool);

  else
    return mysql_connection(mysql_functions_non_blocking<Y>{fiber}, data, put_back_in_pool);
}

} // namespace li