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

namespace li {

template <typename B>
inline mysql_connection<B>::mysql_connection(B mysql_wrapper,
                                             std::shared_ptr<li::mysql_connection_data>& data)
    : mysql_wrapper_(mysql_wrapper), data_(data) {    
}

template <typename B> long long int mysql_connection<B>::last_insert_rowid() {
  return mysql_insert_id(data_->connection_);
}

template <typename B>
sql_result<mysql_result<B>> mysql_connection<B>::operator()(const std::string& rq) {
  mysql_wrapper_.mysql_real_query(data_->error_, data_->connection_, rq.c_str(), rq.size());
 return sql_result<mysql_result<B>>(
      mysql_result<B>(mysql_wrapper_, data_));
}

template <typename B>
template <typename F, typename... K>
mysql_statement<B> mysql_connection<B>::cached_statement(F f, K... keys) {
  if (data_->statements_hashmap_(f).get() == nullptr) {
    mysql_statement<B> res = prepare(f());
    data_->statements_hashmap_(f, keys...) = res.data_.shared_from_this();
    return res;
  } else
    return mysql_statement<B>{mysql_wrapper_, *data_->statements_hashmap_(f, keys...),
                              data_};
}

template <typename B> mysql_statement<B> mysql_connection<B>::prepare(const std::string& rq) {
  auto it = data_->statements_.find(rq);
  if (it != data_->statements_.end()) {
    // mysql_wrapper_.mysql_stmt_free_result(it->second->stmt_);
    // mysql_wrapper_.mysql_stmt_reset(it->second->stmt_);
    return mysql_statement<B>{mysql_wrapper_, *it->second, data_};
  }
  //std::cout << "prepare " << rq << "  "  << data_->statements_.size() << std::endl;
  MYSQL_STMT* stmt = mysql_stmt_init(data_->connection_);
  if (!stmt) {
    data_->error_ = 1;
    throw std::runtime_error(std::string("mysql_stmt_init error: ") +
                             mysql_error(data_->connection_));
  }

  try {
    if (mysql_wrapper_.mysql_stmt_prepare(data_->error_, stmt, rq.data(), rq.size())) {
      data_->error_ = 1;
      throw std::runtime_error(std::string("mysql_stmt_prepare error: ") +
                              mysql_error(data_->connection_));
    }
  } catch (...) {
    mysql_stmt_close(stmt);
    throw;
  }

  auto pair = data_->statements_.emplace(rq, std::make_shared<mysql_statement_data>(stmt));
  return mysql_statement<B>{mysql_wrapper_, *pair.first->second, data_};
}

} // namespace li

#include <li/sql/mysql_connection.hpp>
