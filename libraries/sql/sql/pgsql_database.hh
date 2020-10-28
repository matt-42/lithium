#pragma once

#include <unistd.h>

#include "libpq-fe.h"
#include <atomic>
#include <boost/lexical_cast.hpp>
#include <cassert>
#include <cstring>
#include <deque>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>

#if __linux__
#include <sys/epoll.h>
#elif __APPLE__
#include <sys/event.h>
#endif

#include <thread>
#include <unordered_map>

#include <li/callable_traits/callable_traits.hh>
#include <li/metamap/metamap.hh>
#include <li/sql/pgsql_connection.hh>
#include <li/sql/pgsql_statement.hh>
#include <li/sql/sql_common.hh>
#include <li/sql/sql_database.hh>
#include <li/sql/symbols.hh>
#include <li/sql/type_hashmap.hh>

namespace li {

struct pgsql_database_impl {

  typedef pgsql_connection_data connection_data_type;

  typedef pgsql_tag db_tag;
  std::string host_, user_, passwd_, database_;
  unsigned int port_;
  std::string character_set_;

  template <typename... O> inline pgsql_database_impl(O... opts) {

    auto options = mmm(opts...);
    static_assert(has_key(options, s::host), "open_pgsql_connection requires the s::host argument");
    static_assert(has_key(options, s::database),
                  "open_pgsql_connection requires the s::databaser argument");
    static_assert(has_key(options, s::user), "open_pgsql_connection requires the s::user argument");
    static_assert(has_key(options, s::password),
                  "open_pgsql_connection requires the s::password argument");

    host_ = options.host;
    database_ = options.database;
    user_ = options.user;
    passwd_ = options.password;
    port_ = get_or(options, s::port, 5432);
    character_set_ = get_or(options, s::charset, "utf8");

    if (!PQisthreadsafe())
      throw std::runtime_error("LibPQ is not threadsafe.");
  }

  inline int get_socket(const std::shared_ptr<pgsql_connection_data>& data) {
    return PQsocket(data->pgconn_);
  }

  template <typename Y> inline pgsql_connection_data* new_connection(Y& fiber) {

    PGconn* connection = nullptr;
    int pgsql_fd = -1;
    std::stringstream coninfo;
    coninfo << "postgresql://" << user_ << ":" << passwd_ << "@" << host_ << ":" << port_ << "/"
            << database_;
    // connection = PQconnectdb(coninfo.str().c_str());
    connection = PQconnectStart(coninfo.str().c_str());
    if (!connection) {
      std::cerr << "Warning: PQconnectStart returned null." << std::endl;
      return nullptr;
    }
    if (PQsetnonblocking(connection, 1) == -1) {
      std::cerr << "Warning: PQsetnonblocking returned -1: " << PQerrorMessage(connection)
                << std::endl;
      PQfinish(connection);
      return nullptr;
    }

    int status = PQconnectPoll(connection);

    pgsql_fd = PQsocket(connection);
    if (pgsql_fd == -1) {
      std::cerr << "Warning: PQsocket returned -1: " << PQerrorMessage(connection) << std::endl;
      // If PQsocket return -1, retry later.
      PQfinish(connection);
      return nullptr;
    }
    #if __linux__
      fiber.epoll_add(pgsql_fd, EPOLLIN | EPOLLOUT | EPOLLRDHUP);
    #elif __APPLE__
      fiber.epoll_add(pgsql_fd, EVFILT_READ | EVFILT_WRITE);
    #endif

    try {
      while (status != PGRES_POLLING_FAILED and status != PGRES_POLLING_OK) {
        int new_pgsql_fd = PQsocket(connection);
        if (new_pgsql_fd != pgsql_fd) {
          pgsql_fd = new_pgsql_fd;
          #if __linux__
            fiber.epoll_add(pgsql_fd, EPOLLIN | EPOLLOUT | EPOLLRDHUP);
          #elif __APPLE__
            fiber.epoll_add(pgsql_fd, EVFILT_READ | EVFILT_WRITE);
          #endif
        }
        fiber.yield();
        status = PQconnectPoll(connection);
      }
    } catch (typename Y::exception_type& e) {
      // Yield thrown a exception (probably because a closed connection).
      PQfinish(connection);
      throw std::move(e);
    }
    // std::cout << "CONNECTED " << std::endl;
    #if __linux__
      fiber.epoll_mod(pgsql_fd, EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET);
    #elif __APPLE__
      fiber.epoll_mod(pgsql_fd, EVFILT_READ | EVFILT_WRITE);
    #endif
    if (status != PGRES_POLLING_OK) {
      std::cerr << "Warning: cannot connect to the postgresql server " << host_ << ": "
                << PQerrorMessage(connection) << std::endl;
      PQfinish(connection);
      return nullptr;
    }

    // pgsql_set_character_set(pgsql, character_set_.c_str());
    return new pgsql_connection_data{connection, pgsql_fd};
  }

  template <typename Y>
  auto scoped_connection(Y& fiber, std::shared_ptr<pgsql_connection_data>& data) {
    return pgsql_connection<Y>(fiber, data);
  }
};

typedef sql_database<pgsql_database_impl> pgsql_database;

} // namespace li
