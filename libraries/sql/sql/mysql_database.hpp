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

template <typename... O> inline mysql_database::mysql_database(O... opts) {

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
  port_ = get_or(options, s::port, 0);

  character_set_ = get_or(options, s::charset, "utf8");

  if (mysql_library_init(0, NULL, NULL))
    throw std::runtime_error("Could not initialize MySQL library.");
  if (!mysql_thread_safe())
    throw std::runtime_error("Mysql is not compiled as thread safe.");
}

mysql_database::~mysql_database() { mysql_library_end(); }

template <typename Y>
inline mysql_connection<mysql_functions_non_blocking<Y>> mysql_database::connect(Y& fiber) {

  // std::cout << "nconnection " << total_number_of_mysql_connections << std::endl;
  int ntry = 0;
  std::shared_ptr<mysql_connection_data> data = nullptr;
  while (!data) {
    // if (ntry > 20)
    //   throw std::runtime_error("Cannot connect to the database");
    ntry++;

    if (!mysql_connection_async_pool.empty()) {
      data = mysql_connection_async_pool.back();
      mysql_connection_async_pool.pop_back();
      fiber.epoll_add(mysql_get_socket(data->connection_),
                      EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET);
    } else {
      // std::cout << total_number_of_mysql_connections << " connections. "<< std::endl;
      if (total_number_of_mysql_connections > max_mysql_connections_per_thread) {
        // std::cout << "Waiting for a free mysql connection..." << std::endl;
        fiber.yield();
        continue;
      }
      total_number_of_mysql_connections++;
      // std::cout << "NEW MYSQL CONNECTION "  << std::endl;
      MYSQL* mysql;
      int mysql_fd = -1;
      int status;
      MYSQL* connection;
      // while (mysql_fd == -1)
      {
        mysql = new MYSQL;
        mysql_init(mysql);
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
          total_number_of_mysql_connections--;
          // usleep(1e6);
          fiber.yield();
          continue;
        }
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
          total_number_of_mysql_connections--;
          mysql_close(mysql);
          throw std::move(e);
        }
      if (!connection) {
        // std::cout << "Error in mysql_real_connect_cont" << std::endl;
        total_number_of_mysql_connections--;
        fiber.yield();
        continue;
      }
      // throw std::runtime_error("Cannot connect to the database");
      mysql_set_character_set(mysql, character_set_.c_str());
      data = std::shared_ptr<mysql_connection_data>(new mysql_connection_data{mysql});
    }
  }
  assert(data);
  return std::move(mysql_connection(mysql_functions_non_blocking<Y>{fiber}, data));
}

inline mysql_connection<mysql_functions_blocking> mysql_database::connect() {
  std::shared_ptr<mysql_connection_data> data = nullptr;
  if (!mysql_connection_pool.empty()) {
    data = mysql_connection_pool.back();
    mysql_connection_pool.pop_back();
  }

  if (!data) {
    MYSQL* con_ = mysql_init(nullptr);
    con_ = mysql_real_connect(con_, host_.c_str(), user_.c_str(), passwd_.c_str(),
                              database_.c_str(), port_, NULL, 0);
    if (!con_)
      throw std::runtime_error("Cannot connect to the database");

    // total_number_of_mysql_connections++;
    mysql_set_character_set(con_, character_set_.c_str());
    data = std::shared_ptr<mysql_connection_data>(new mysql_connection_data{con_});
  }

  assert(data);
  return std::move(mysql_connection(mysql_functions_blocking{}, data));
}

} // namespace li