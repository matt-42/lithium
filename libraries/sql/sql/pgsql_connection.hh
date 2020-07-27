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
#include <li/sql/pgsql_result.hh>
#include <li/sql/pgsql_statement.hh>
#include <li/sql/sql_common.hh>
#include <li/sql/symbols.hh>
#include <li/sql/type_hashmap.hh>
#include <li/sql/pgsql_connection_data.hh>

namespace li {

struct pgsql_tag {};

// template <typename Y> void pq_wait(Y& yield, PGconn* con) {
//   while (PQisBusy(con))
//     yield();
// }

template <typename Y> struct pgsql_connection {

  Y& fiber_;
  std::shared_ptr<pgsql_connection_data> data_;
  std::unordered_map<std::string, std::shared_ptr<pgsql_statement_data>>& stm_cache_;
  PGconn* connection_;

  typedef pgsql_tag db_tag;

  inline pgsql_connection(const pgsql_connection&) = delete;
  inline pgsql_connection& operator=(const pgsql_connection&) = delete;
  inline pgsql_connection(pgsql_connection&& o) = default;

  inline pgsql_connection(Y& fiber, std::shared_ptr<pgsql_connection_data>& data)
      : fiber_(fiber), data_(data), stm_cache_(data->statements), connection_(data->pgconn_) {

  }

  // FIXME long long int last_insert_rowid() { return pgsql_insert_id(connection_); }

  // pgsql_statement<Y> operator()(const std::string& rq) { return prepare(rq)(); }

  auto operator()(const std::string& rq) {
    if (!PQsendQueryParams(connection_, rq.c_str(), 0, nullptr, nullptr, nullptr, nullptr, 1))
      throw std::runtime_error(std::string("Postresql error:") + PQerrorMessage(connection_));
    return sql_result<pgsql_result<Y>>{
        pgsql_result<Y>{this->data_, this->fiber_, data_->error_}};
  }

  // PQsendQueryParams
  template <typename F, typename... K> pgsql_statement<Y> cached_statement(F f, K... keys) {
    if (data_->statements_hashmap(f, keys...).get() == nullptr) {
      pgsql_statement<Y> res = prepare(f());
      data_->statements_hashmap(f, keys...) = res.data_.shared_from_this();
      return res;
    } else
      return pgsql_statement<Y>{data_, fiber_, *data_->statements_hashmap(f, keys...)};
  }

  pgsql_statement<Y> prepare(const std::string& rq) {
    auto it = stm_cache_.find(rq);
    if (it != stm_cache_.end()) {
      return pgsql_statement<Y>{data_, fiber_, *it->second};
    }
    std::string stmt_name = boost::lexical_cast<std::string>(stm_cache_.size());

    if (!PQsendPrepare(connection_, stmt_name.c_str(), rq.c_str(), 0, nullptr)) {
      throw std::runtime_error(std::string("PQsendPrepare error") + PQerrorMessage(connection_));
    }

    // flush results.
    while (PGresult* ret = pg_wait_for_next_result(connection_, fiber_, data_->error_))
      PQclear(ret);

    auto pair = stm_cache_.emplace(rq, std::make_shared<pgsql_statement_data>(stmt_name));
    return pgsql_statement<Y>{data_, fiber_, *pair.first->second};
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
