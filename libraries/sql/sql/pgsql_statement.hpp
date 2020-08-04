#pragma once

#include <libpq-fe.h>

#include <li/callable_traits/callable_traits.hh>
#include <li/metamap/metamap.hh>
#include <li/sql/sql_common.hh>
#include <li/sql/symbols.hh>
#include <memory>

namespace li {

// Execute a request with placeholders.
template <typename Y>
template <unsigned N>
void pgsql_statement<Y>::bind_param(sql_varchar<N>&& m, const char** values, int* lengths,
                                    int* binary) {
  // std::cout << "send param varchar " << m << std::endl;
  *values = m.c_str();
  *lengths = m.size();
  *binary = 0;
}
template <typename Y>
template <unsigned N>
void pgsql_statement<Y>::bind_param(const sql_varchar<N>& m, const char** values, int* lengths,
                                    int* binary) {
  // std::cout << "send param const varchar " << m << std::endl;
  *values = m.c_str();
  *lengths = m.size();
  *binary = 0;
}
template <typename Y>
void pgsql_statement<Y>::bind_param(const char* m, const char** values, int* lengths, int* binary) {
  // std::cout << "send param const char*[N] " << m << std::endl;
  *values = m;
  *lengths = strlen(m);
  *binary = 0;
}

template <typename Y>
template <typename T>
void pgsql_statement<Y>::bind_param(const std::vector<T>& m, const char** values, int* lengths,
                                    int* binary) {
  int tsize = [&] {
    if constexpr (is_metamap<T>::value)
      return metamap_size<T>();
    else
      return 1;
  }();

  int i = 0;
  for (int i = 0; i < m.size(); i++)
    bind_param(m[i], values + i * tsize, lengths + i * tsize, binary + i * tsize);
}

template <typename Y>
template <typename T>
void pgsql_statement<Y>::bind_param(const T& m, const char** values, int* lengths, int* binary) {
  if constexpr (is_metamap<std::decay_t<decltype(m)>>::value) {
    int i = 0;
    li::map(m, [&](auto k, const auto& m) {
      bind_param(m, values + i, lengths + i, binary + i);
      i++;
    });
  } else if constexpr (std::is_same<std::decay_t<decltype(m)>, std::string>::value or
                       std::is_same<std::decay_t<decltype(m)>, std::string_view>::value) {
    // std::cout << "send param string: " << m << std::endl;
    *values = m.c_str();
    *lengths = m.size();
    *binary = 0;
  } else if constexpr (std::is_same<std::remove_reference_t<decltype(m)>, const char*>::value) {
    // std::cout << "send param const char* " << m << std::endl;
    *values = m;
    *lengths = strlen(m);
    *binary = 0;
  } else if constexpr (std::is_same<std::decay_t<decltype(m)>, int>::value) {
    *values = (char*)new int(htonl(m));
    *lengths = sizeof(m);
    *binary = 1;
  } else if constexpr (std::is_same<std::decay_t<decltype(m)>, long long int>::value) {
    // FIXME send 64bit values.
    // std::cout << "long long int param: " << m << std::endl;
    *values = (char*)new int(htonl(uint32_t(m)));
    *lengths = sizeof(uint32_t);
    // does not work:
    // values = (char*)new uint64_t(htobe64((uint64_t) m));
    // lengths = sizeof(uint64_t);
    *binary = 1;
  }
}

template <typename Y>
template <typename T>
void pgsql_statement<Y>::free_bind_param(const T& m, const char** values) {
  if constexpr (is_metamap<std::decay_t<decltype(m)>>::value) {
    int i = 0;
    li::map(m, [&](auto k, const auto& m) {
      free_bind_param(m, values + i);
      i++;
    });
  } else if constexpr (std::is_same<std::decay_t<decltype(m)>, std::string>::value or
                       std::is_same<std::decay_t<decltype(m)>, std::string_view>::value) {
  } else if constexpr (std::is_same<std::remove_reference_t<decltype(m)>, const char*>::value) {
  } else if constexpr (std::is_same<std::decay_t<decltype(m)>, int>::value) {
    delete *values;
  } else if constexpr (std::is_same<std::decay_t<decltype(m)>, long long int>::value) {
    delete *values;
  }
}

template <typename Y>
template <typename T>
unsigned int pgsql_statement<Y>::bind_compute_nparam(const T& arg) {
  return 1;
}
template <typename Y>
template <typename... T>
unsigned int pgsql_statement<Y>::bind_compute_nparam(const metamap<T...>& arg) {
  return sizeof...(T);
}
template <typename Y>
template <typename T>
unsigned int pgsql_statement<Y>::bind_compute_nparam(const std::vector<T>& arg) {
  return arg.size() * bind_compute_nparam(arg[0]);
}

// Bind parameter to the prepared statement and execute it.
template <typename Y>
template <typename... T>
sql_result<pgsql_result<Y>> pgsql_statement<Y>::operator()(T&&... args) {

  unsigned int nparams = 0;
  if constexpr (sizeof...(T) > 0)
    nparams = (bind_compute_nparam(std::forward<T>(args)) + ...);
  const char* values_[nparams];
  int lengths_[nparams];
  int binary_[nparams];

  const char** values = values_;
  int* lengths = lengths_;
  int* binary = binary_;

  int i = 0;
  tuple_map(std::forward_as_tuple(args...), [&](const auto& a) {
    bind_param(a, values + i, lengths + i, binary + i);
    i += bind_compute_nparam(a);
  });

  // std::cout << "PQsendQueryPrepared" << std::endl; 
  if (!PQsendQueryPrepared(connection_->pgconn_, data_.stmt_name.c_str(), nparams, values, lengths, binary,
                           1)) {
    throw std::runtime_error(std::string("Postresql error:") + PQerrorMessage(connection_->pgconn_));
  }

  i = 0;
  tuple_map(std::forward_as_tuple(args...), [&](const auto& a) {
    free_bind_param(a, values + i);
    i += bind_compute_nparam(a);
  });

  // Not calling pqflush seems to work aswell...
  // connection_->flush(this->fiber_);

  // this->connection_->batch_query(this->fiber_, true);
  int query_result_id = this->connection_->batch_query(this->fiber_);
  // println("build result ", query_result_id);
  return sql_result<pgsql_result<Y>>{
      pgsql_result<Y>(this->connection_, this->fiber_, query_result_id)};
}

// FIXME long long int affected_rows() { return pgsql_stmt_affected_rows(data_.stmt_); }

} // namespace li
