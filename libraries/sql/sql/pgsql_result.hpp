#pragma once

#include "libpq-fe.h"
#include <li/sql/internal/utils.hh>
//#include <catalog/pg_type_d.h>

#define INT8OID 20
#define INT2OID 21
#define INT4OID 23

namespace li {

template <typename Y>
PGresult* pg_wait_for_next_result(PGconn* connection, Y& fiber,
                                  std::shared_ptr<int> connection_status, bool nothrow = false) {
  // std::cout << "WAIT ======================" << std::endl;
  while (true) {
    if (PQconsumeInput(connection) == 0)
    {
      if (!nothrow)
        throw std::runtime_error(std::string("PQconsumeInput() failed: ") +
                                 PQerrorMessage(connection));
      else
        std::cerr << "PQconsumeInput() failed: " << PQerrorMessage(connection) << std::endl;
    }

    if (PQisBusy(connection)) {
      // std::cout << "isbusy" << std::endl;
      try {
        fiber.yield();
      } catch (typename Y::exception_type& e) {
        // Yield thrown a exception (probably because a closed connection).
        // Mark the connection as broken because it is left in a undefined state.
        *connection_status = 1;
        throw std::move(e);
      }
    } else {
      // std::cout << "notbusy" << std::endl;
      PGresult* res = PQgetResult(connection);
      if (PQresultStatus(res) == PGRES_FATAL_ERROR and PQerrorMessage(connection)[0] != 0)
      {
        *connection_status = 1;          
        if (!nothrow)
          throw std::runtime_error(std::string("Postresql fatal error:") +
                                  PQerrorMessage(connection));
        else
          std::cerr << "Postgresql FATAL error: " << PQerrorMessage(connection) << std::endl;

      }
      else if (PQresultStatus(res) == PGRES_NONFATAL_ERROR)
        std::cerr << "Postgresql non fatal error: " << PQerrorMessage(connection) << std::endl;

      return res;
    }
  }
}
template <typename Y> PGresult* pgsql_result<Y>::wait_for_next_result() {
  return pg_wait_for_next_result(connection_, fiber_, connection_status_);
}

template <typename Y> void pgsql_result<Y>::flush_results() {
  try {
    while (true)
    {
      if (*connection_status_ == 1) break;
      PGresult* res = pg_wait_for_next_result(connection_, fiber_, connection_status_, true);
      if (res)
        PQclear(res);
      else break;
    }
  } catch (typename Y::exception_type& e) {
    // Forward fiber execptions.
    throw std::move(e);
  }
}

// Fetch a string from a result field.
template <typename Y>
template <typename... A>
void pgsql_result<Y>::fetch_value(std::string& out, char* val, int length, bool is_binary,
                                  Oid field_type) {
  // assert(!is_binary);
  // std::cout << "fetch string: " << length << " '"<< val <<"'" << std::endl;
  out = std::move(std::string(val, strlen(val)));
}

// Fetch a blob from a result field.
template <typename Y>
template <typename... A>
void pgsql_result<Y>::fetch_value(sql_blob& out, char* val, int length, bool is_binary,
                                  Oid field_type) {
  // assert(is_binary);
  out = std::move(std::string(val, length));
}

// Fetch an int from a result field.
template <typename Y>
void pgsql_result<Y>::fetch_value(int& out, char* val, int length, bool is_binary, Oid field_type) {
  assert(is_binary);
  // TYPCATEGORY_NUMERIC
  // std::cout << "fetch integer " << length << " " << is_binary << std::endl;
  // std::cout << "fetch integer " << be64toh(*((uint64_t *) val)) << std::endl;
  if (field_type == INT8OID) {
    // std::cout << "fetch 64b integer " << std::hex << int(32) << std::endl;
    // std::cout << "fetch 64b integer " << std::hex << uint64_t(*((uint64_t *) val)) << std::endl;
    // std::cout << "fetch 64b integer " << std::hex << (*((uint64_t *) val)) << std::endl;
    // std::cout << "fetch 64b integer " << std::hex << be64toh(*((uint64_t *) val)) << std::endl;
    out = be64toh(*((uint64_t*)val));
  } else if (field_type == INT4OID)
    out = (uint32_t)ntohl(*((uint32_t*)val));
  else if (field_type == INT2OID)
    out = (uint16_t)ntohs(*((uint16_t*)val));
  else
    throw std::runtime_error("The type of request result does not match the destination type");
}

// Fetch an unsigned int from a result field.
template <typename Y>
void pgsql_result<Y>::fetch_value(unsigned int& out, char* val, int length, bool is_binary,
                                  Oid field_type) {
  assert(is_binary);
  // if (length == 8)
  if (field_type == INT8OID)
    out = be64toh(*((uint64_t*)val));
  else if (field_type == INT4OID)
    out = ntohl(*((uint32_t*)val));
  else if (field_type == INT2OID)
    out = ntohs(*((uint16_t*)val));
  else
    assert(0);
}

template <typename B> template <typename T> bool pgsql_result<B>::read(T&& output) {

  if (!current_result_ || row_i_ == current_result_nrows_) {
    if (current_result_) {
      PQclear(current_result_);
      current_result_ = nullptr;
    }
    current_result_ = wait_for_next_result();
    if (!current_result_)
      return false;
    row_i_ = 0;
    current_result_nrows_ = PQntuples(current_result_);
    if (current_result_nrows_ == 0) {
      PQclear(current_result_);
      current_result_ = nullptr;
      return false;
    }

    if (curent_result_field_types_.size() == 0) {
      curent_result_field_types_.resize(PQnfields(current_result_));
      for (int field_i = 0; field_i < curent_result_field_types_.size(); field_i++)
        curent_result_field_types_[field_i] = PQftype(current_result_, field_i);
    }
  }

  // Tuples
  if constexpr (is_tuple<T>::value) {
    int field_i = 0;

    int nfields = curent_result_field_types_.size();
    if (nfields != std::tuple_size_v<std::decay_t<T>>)
      throw std::runtime_error("postgresql error: in fetch: Mismatch between the request number of "
                               "field and the outputs.");

    tuple_map(std::forward<T>(output), [&](auto& m) {
      fetch_value(m, PQgetvalue(current_result_, row_i_, field_i),
                  PQgetlength(current_result_, row_i_, field_i),
                  PQfformat(current_result_, field_i), curent_result_field_types_[field_i]);
      field_i++;
    });
  } else { // Metamaps.
    li::map(std::forward<T>(output), [&](auto k, auto& m) {
      int field_i = PQfnumber(current_result_, symbol_string(k));
      if (field_i == -1)
        throw std::runtime_error(std::string("postgresql errror : Field ") + symbol_string(k) +
                                 " not fount in result.");

      fetch_value(m, PQgetvalue(current_result_, row_i_, field_i),
                  PQgetlength(current_result_, row_i_, field_i),
                  PQfformat(current_result_, field_i), curent_result_field_types_[field_i]);
    });
  }

  this->row_i_++;

  return true;
}

// Get the last id of the row inserted by the last command.
template <typename Y> long long int pgsql_result<Y>::last_insert_id() {
  // while (PGresult* res = wait_for_next_result())
  //  PQclear(res);
  // PQsendQuery(connection_, "LASTVAL()");
  int t = 0;
  this->read(std::tie(t));
  return t;
  // PGresult *PQexec(connection_, const char *command);
  // this->operator()
  //   last_insert_id_ = PQoidValue(res);
  //   std::cout << "id " << last_insert_id_ << std::endl;
  //   PQclear(res);
  // }
  // return last_insert_id_;
}

} // namespace li