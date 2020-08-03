#pragma once

#include <arpa/inet.h>
#include "libpq-fe.h"
#include <li/sql/internal/utils.hh>
//#include <catalog/pg_type_d.h>

#define INT8OID 20
#define INT2OID 21
#define INT4OID 23

namespace li {

template <typename Y>
PGresult* pg_wait_for_next_result(PGconn* connection, Y& fiber,
                                  int& connection_status, bool nothrow = false) {
  // std::cout << "WAIT ======================" << std::endl;
  while (true) {
    if (PQconsumeInput(connection) == 0)
    {
      connection_status = 1;          
      if (!nothrow)
        throw std::runtime_error(std::string("PQconsumeInput() failed: ") +
                                 PQerrorMessage(connection));
      else
        std::cerr << "PQconsumeInput() failed: " << PQerrorMessage(connection) << std::endl;
#ifdef DEBUG
      assert(0);
#endif
    }

    if (PQisBusy(connection)) {
      // std::cout << "isbusy" << std::endl;
      try {
        fiber.yield();
      } catch (typename Y::exception_type& e) {
        // Free results.
        // Yield thrown a exception (probably because a closed connection).
        // Flush the remaining results.
        while (true)
        {
          if (PQconsumeInput(connection) == 0)
          {
            connection_status = 1;
            break;
          }
          if (!PQisBusy(connection))
          {
            PGresult* res = PQgetResult(connection);
            if (res) PQclear(res);
            else break;
          }
        }
        throw std::move(e);
      }
    } else {
      // std::cout << "notbusy" << std::endl;
      PGresult* res = PQgetResult(connection);
      auto status = PQresultStatus(res);
      if (status == PGRES_FATAL_ERROR and PQerrorMessage(connection)[0] != 0)
      {
        PQclear(res);
        connection_status = 1;          
        if (!nothrow)
          throw std::runtime_error(std::string("Postresql fatal error:") +
                                  PQerrorMessage(connection));
        else
          std::cerr << "Postgresql FATAL error: " << PQerrorMessage(connection) << std::endl;
#ifdef DEBUG
        assert(0);
#endif
      }
      else if (status == PGRES_NONFATAL_ERROR)
      {
        PQclear(res);
        std::cerr << "Postgresql non fatal error: " << PQerrorMessage(connection) << std::endl;
      }        
      else if (status == PGRES_BATCH_ABORTED) {
        PQclear(res);
        std::cerr << "Postgresql batch aborted: " << PQerrorMessage(connection) << std::endl;        
      }
      return res;
    }
  }
}
template <typename Y> PGresult* pgsql_result<Y>::wait_for_next_result() {
  return pg_wait_for_next_result(connection_->pgconn_, fiber_, connection_->error_);
}

template <typename Y> void pgsql_result<Y>::flush_results() {
  if (end_of_result_) return;

  if (connection_->current_result_id == -1)
    this->connection_->end_of_current_result(fiber_);

  while (connection_->current_result_id != this->result_id_)
    this->fiber_.yield();

  try {
    while (true)
    {
      if (connection_->error_ == 1) break;
      PGresult* res = pg_wait_for_next_result(connection_->pgconn_, fiber_, connection_->error_, true);
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
void pgsql_result<Y>::fetch_value(std::string& out, int field_i,
                                  Oid field_type) {
  // assert(!is_binary);
  // std::cout << "fetch string: " << length << " '"<< val <<"'" << std::endl;
  out = std::move(std::string(PQgetvalue(current_result_, row_i_, field_i),
                              PQgetlength(current_result_, row_i_, field_i)));
  // out = std::move(std::string(val, strlen(val)));
}

// Fetch a blob from a result field.
template <typename Y>
template <typename... A>
void pgsql_result<Y>::fetch_value(sql_blob& out, int field_i, Oid field_type) {
  // assert(is_binary);
  out = std::move(std::string(PQgetvalue(current_result_, row_i_, field_i),
                              PQgetlength(current_result_, row_i_, field_i)));
}

// Fetch an int from a result field.
template <typename Y>
void pgsql_result<Y>::fetch_value(int& out, int field_i, Oid field_type) {
  assert(PQfformat(current_result_, field_i) == 1); // Assert binary format
  char* val = PQgetvalue(current_result_, row_i_, field_i);

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
void pgsql_result<Y>::fetch_value(unsigned int& out, int field_i, Oid field_type) {
  assert(PQfformat(current_result_, field_i) == 1); // Assert binary format
  char* val = PQgetvalue(current_result_, row_i_, field_i);

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


template <typename B> template <typename T> bool pgsql_result<B>::fetch_next_result(T&& output) {

  if (end_of_result_) return false;

  if (connection_->current_result_id == -1)
  {
    this->connection_->end_of_current_result(fiber_);
  }

  // std::cout << "read: fiber_.continuation_idx " << fiber_.continuation_idx << std::endl;
  // std::cout << "connection_->current_result_id " << connection_->current_result_id << " this->result_id_ " << this->result_id_ << std::endl;

  while (connection_->current_result_id != this->result_id_)
  {
    // std::cout << "read: fiber_.continuation_idx " << fiber_.continuation_idx << std::endl;
    // std::cout << "connection_->current_result_id " << connection_->current_result_id << " this->result_id_ " << this->result_id_ << std::endl;
    this->fiber_.yield();
  }

  // std::cout << "GO read: fiber_.continuation_idx " << fiber_.continuation_idx << std::endl;
  // std::cout << "GO connection_->current_result_id " << connection_->current_result_id << " this->result_id_ " << this->result_id_ << std::endl;
  assert(connection_->current_result_id == this->result_id_);

  if (!current_result_ || row_i_ == current_result_nrows_) {

    current_result_nrows_ = 0;
    while (current_result_nrows_ == 0)
    {
      if (current_result_) {
        PQclear(current_result_);
        current_result_ = nullptr;
      }

      current_result_ = wait_for_next_result();
      assert(connection_->current_result_id == this->result_id_);
      if (!current_result_)
      {
        this->connection_->end_of_current_result(fiber_);
        // std::cout << " connection_->current_result_id " << connection_->current_result_id << " this->result_id_: " << this->result_id_ << std::endl;
        // assert(connection_->current_result_id != this->result_id_);
        end_of_result_ = true; 
        return false;
      }
      row_i_ = 0;
      current_result_nrows_ = PQntuples(current_result_);
    }

    // Metamaps.
    if constexpr (is_metamap<std::decay_t<T>>::value) {
      curent_result_field_positions_.clear();
      li::map(std::forward<T>(output), [&](auto k, auto& m) {
        curent_result_field_positions_.push_back(PQfnumber(current_result_, symbol_string(k)));
        if (curent_result_field_positions_.back() == -1)
          throw std::runtime_error(std::string("postgresql errror : Field ") + symbol_string(k) +
                                    " not found in result.");

      });
    }

    if (curent_result_field_types_.size() == 0) {

      curent_result_field_types_.resize(PQnfields(current_result_));
      for (int field_i = 0; field_i < curent_result_field_types_.size(); field_i++)
        curent_result_field_types_[field_i] = PQftype(current_result_, field_i);
    }
  }
  return true;
}

template <typename B> template <typename T> bool pgsql_result<B>::read(T&& output) {

  if (!current_result_ && !this->fetch_next_result<T>(std::forward<T>(output))) return false;

  assert(connection_->current_result_id == this->result_id_);

  // std::cout << "got a row! " << std::endl;

  // Tuples
  if constexpr (is_tuple<T>::value) {
    int field_i = 0;

    int nfields = curent_result_field_types_.size();
    if (nfields != std::tuple_size_v<std::decay_t<T>>)
      throw std::runtime_error("postgresql error: in fetch: Mismatch between the request number of "
                               "field and the outputs.");

    tuple_map(std::forward<T>(output), [&](auto& m) {
      fetch_value(m, field_i, curent_result_field_types_[field_i]);
      field_i++;
    });
  } else { // Metamaps.
    int i = 0;
    li::map(std::forward<T>(output), [&](auto k, auto& m) {
      int field_i = curent_result_field_positions_[i];
      fetch_value(m, field_i, curent_result_field_types_[field_i]);
      i++;
    });
  }

  this->row_i_++;
  if (this->row_i_ == current_result_nrows_) this->fetch_next_result<T>(std::forward<T>(output));
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
