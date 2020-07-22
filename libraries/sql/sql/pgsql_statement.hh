#pragma once

#include <libpq-fe.h>

#include <li/callable_traits/callable_traits.hh>
#include <li/metamap/metamap.hh>
#include <li/sql/sql_common.hh>
#include <li/sql/symbols.hh>
#include <memory>
#include <li/sql/pgsql_result.hh>
#include <li/sql/sql_result.hh>

namespace li {

struct pgsql_statement_data : std::enable_shared_from_this<pgsql_statement_data> {
  pgsql_statement_data(const std::string& s) : stmt_name(s) {}
  std::string stmt_name;
};

template <typename Y> struct pgsql_statement {

public:
  template <typename... T> sql_result<pgsql_result<Y>> operator()(T&&... args);

  std::shared_ptr<pgsql_connection_data> connection_;
  Y& fiber_;
  pgsql_statement_data& data_;
  int& connection_status_;


private:

  // Bind statement param utils.
  template <unsigned N>
  void bind_param(sql_varchar<N>&& m, const char** values, int* lengths, int* binary);
  template <unsigned N>
  void bind_param(const sql_varchar<N>& m, const char** values, int* lengths, int* binary);
  void bind_param(const char* m, const char** values, int* lengths, int* binary);
  template <typename T>
  void bind_param(const std::vector<T>& m, const char** values, int* lengths, int* binary);
  template <typename T> void bind_param(const T& m, const char** values, int* lengths, int* binary);
  template <typename T> unsigned int bind_compute_nparam(const T& arg);
  template <typename... T> unsigned int bind_compute_nparam(const metamap<T...>& arg);
  template <typename T> unsigned int bind_compute_nparam(const std::vector<T>& arg);
  // Bind parameter to the prepared statement and execute it.
  // FIXME long long int affected_rows() { return pgsql_stmt_affected_rows(data_.stmt_); }

};

} // namespace li

#include <li/sql/pgsql_statement.hpp>