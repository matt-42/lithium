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
#include <sys/epoll.h>
#include <thread>
#include <unordered_map>

#include <li/callable_traits/callable_traits.hh>
#include <li/metamap/metamap.hh>
#include <li/sql/pgsql_statement.hh>
#include <li/sql/pgsql_result.hh>
#include <li/sql/sql_common.hh>
#include <li/sql/symbols.hh>
#include <li/sql/type_hashmap.hh>

namespace li {

struct pgsql_tag {};

struct pgsql_database;

struct pgsql_connection_data {

  ~pgsql_connection_data() {
    if (connection) {
      cancel();
      PQfinish(connection);
    }
  }
  void cancel() {
    if (connection) {
      // Cancel any pending request.
      PGcancel* cancel = PQgetCancel(connection);
      char x[256];
      if (cancel) {
        PQcancel(cancel, x, 256);
        PQfreeCancel(cancel);
      }
    }
  }

  PGconn* connection = nullptr;
  int fd = -1;
  std::unordered_map<std::string, std::shared_ptr<pgsql_statement_data>> statements;
  type_hashmap<std::shared_ptr<pgsql_statement_data>> statements_hashmap;
};

thread_local std::deque<std::shared_ptr<pgsql_connection_data>> pgsql_connection_pool;

// template <typename Y> void pq_wait(Y& yield, PGconn* con) {
//   while (PQisBusy(con))
//     yield();
// }

template <typename Y> struct pgsql_connection {

  Y& fiber_;
  std::shared_ptr<pgsql_connection_data> data_;
  std::unordered_map<std::string, std::shared_ptr<pgsql_statement_data>>& stm_cache_;
  PGconn* connection_;
  std::shared_ptr<int> connection_status_;

  typedef pgsql_tag db_tag;

  inline pgsql_connection(const pgsql_connection&) = delete;
  inline pgsql_connection& operator=(const pgsql_connection&) = delete;
  inline pgsql_connection(pgsql_connection&&) = default;

  template <typename P>
  inline pgsql_connection(Y& fiber, std::shared_ptr<li::pgsql_connection_data> data,
                          P put_data_back_in_pool)
      : fiber_(fiber), data_(data), stm_cache_(data->statements), connection_(data->connection) {

    connection_status_ = std::shared_ptr<int>(
        new int(0), [data](int* p) mutable { put_data_back_in_pool(data, *p); });
  }

  ~pgsql_connection() {
    if (connection_status_ && *connection_status_ == 0) {
      // flush results if needed.
      // while (PGresult* res = wait_for_next_result())
      //   PQclear(res);
    }
  }

  // FIXME long long int last_insert_rowid() { return pgsql_insert_id(connection_); }

  // pgsql_statement<Y> operator()(const std::string& rq) { return prepare(rq)(); }

  auto& operator()(const std::string& rq) {
    if (!PQsendQuery(connection_, rq.c_str()))
      throw std::runtime_error(std::string("Postresql error:") + PQerrorMessage(connection_));
    return sql_result<pgsql_result<Y>>{
        pgsql_result<Y>{this->connection_, this->fiber_, this->connection_status_}};
  }

  template <typename F, typename... K> pgsql_statement<Y> cached_statement(F f, K... keys) {
    if (data_->statements_hashmap(f, keys...).get() == nullptr) {
      pgsql_statement<Y> res = prepare(f());
      data_->statements_hashmap(f, keys...) = res.data_.shared_from_this();
      return res;
    } else
      return pgsql_statement<Y>{connection_, fiber_, *data_->statements_hashmap(f, keys...),
                                connection_status_};
  }

  pgsql_statement<Y> prepare(const std::string& rq) {
    auto it = stm_cache_.find(rq);
    if (it != stm_cache_.end()) {
      // pgsql_wrapper_.pgsql_stmt_free_result(it->second->stmt_);
      // pgsql_wrapper_.pgsql_stmt_reset(it->second->stmt_);
      return pgsql_statement<Y>{connection_, fiber_, *it->second, connection_status_};
    }
    std::stringstream stmt_name;
    stmt_name << (void*)connection_ << stm_cache_.size();
    // std::cout << "prepare " << rq << " NAME: " << stmt_name.str() << std::endl;

    // FIXME REALLY USEFUL??
    // while (PGresult* res = wait_for_next_result())
    //   PQclear(res);

    if (!PQsendPrepare(connection_, stmt_name.str().c_str(), rq.c_str(), 0, nullptr)) {
      throw std::runtime_error(std::string("PQsendPrepare error") + PQerrorMessage(connection_));
    }

    while (PGresult* ret = pg_wait_for_next_result(connection_, fiber_, connection_status_))
      PQclear(ret);

    // flush results.
    // while (PGresult* ret = PQgetResult(connection_)) {
    //   if (PQresultStatus(ret) == PGRES_FATAL_ERROR)
    //     throw std::runtime_error(std::string("Postresql fatal error:") +
    //                              PQerrorMessage(connection_));
    //   if (PQresultStatus(ret) == PGRES_NONFATAL_ERROR)
    //     std::cerr << "Postgresql non fatal error: " << PQerrorMessage(connection_) << std::endl;
    //   PQclear(ret);
    // }
    // pq_wait(yield_, connection_);

    auto pair = stm_cache_.emplace(rq, std::make_shared<pgsql_statement_data>(stmt_name.str()));
    return pgsql_statement<Y>{connection_, fiber_, *pair.first->second, connection_status_};
  }

  template <typename T>
  inline std::string type_to_string(const T&, std::enable_if_t<std::is_integral<T>::value>* = 0) {
    return "INT";
  }
  template <typename T>
  inline std::string type_to_string(const T&,
                                    std::enable_if_t<std::is_floating_point<T>::value>* = 0) {
    return "DOUBLE";
  }
  inline std::string type_to_string(const std::string&) { return "TEXT"; }
  inline std::string type_to_string(const sql_blob&) { return "BLOB"; }
  template <unsigned S> inline std::string type_to_string(const sql_varchar<S>) {
    std::ostringstream ss;
    ss << "VARCHAR(" << S << ')';
    return ss.str();
  }
};

} // namespace li

#include <li/sql/pgsql_database.hh>
