#pragma once

#include <arpa/inet.h>
#include "libpq-fe.h"
#include <li/sql/internal/utils.hh>
//#include <catalog/pg_type_d.h>

#define INT8OID 20
#define INT2OID 21
#define INT4OID 23

namespace li {

template <typename Y> PGresult* pgsql_result<Y>::wait_for_next_result() {
  return connection_->wait_for_next_result(fiber_);
}

template <typename Y> void pgsql_result<Y>::flush_results() {
  if (end_of_result_) return;
  // println("pgsql_result<Y>::flush_results ", this->result_id_);
  this->connection_->wait_for_result(fiber_, this->result_id_);

  // println("pgsql_result<Y>::flush_results result ready for flush.");

  // try {
    if (current_result_)
    {
      PQclear(current_result_);
      current_result_ = nullptr;
    }
    while (true)
    {
      if (connection_->error_ == 1) break;
      PGresult* res = connection_->wait_for_next_result(fiber_, true);
      if (res)
        PQclear(res);
      else break;
    }
  // } catch (typename Y::exception_type& e) {
  //   // Forward fiber execptions.
  //   throw std::move(e);
  // }

  // println("pgsql_result<Y>::flush_results OK ", this->result_id_);
  end_of_result_ = true; 
  this->connection_->end_of_current_result(fiber_);
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

  // println("pgsql_result<B>::fetch_next_result", this->result_id_);
  if (end_of_result_) return false;
  // std::cout << "read: fiber_.continuation_idx " << fiber_.continuation_idx << std::endl;
  // std::cout << "connection_->current_result_id " << connection_->current_result_id << " this->result_id_ " << this->result_id_ << std::endl;

  // if (!current_result_)
  connection_->wait_for_result(fiber_, this->result_id_);

  // std::cout << "GO read: fiber_.continuation_idx " << fiber_.continuation_idx << std::endl;
  // std::cout << "GO connection_->current_result_id " << connection_->current_result_id << " this->result_id_ " << this->result_id_ << std::endl;
  assert(connection_->current_result_id == this->result_id_);
  // std::cout << "currently reading result " << this->result_id_ << std::endl;
  if (current_result_ && row_i_ < current_result_nrows_ - 1)
  {
    this->row_i_++;
  }
  else {

    current_result_nrows_ = 0;
    while (current_result_nrows_ == 0)
    {
      if (current_result_) {
        PQclear(current_result_);
        current_result_ = nullptr;
      }

      assert(connection_->current_result_id == this->result_id_);
      current_result_ = wait_for_next_result();
      // println("got next result ", current_result_);
      // std::cout <<
      if (connection_->current_result_id_ != this->result_id_) 
      {
        std::cout << " in fetch_next_result: fiberid " << fiber_.continuation_idx << std::endl;
        std::cout << " connection_->current_result_id " << connection_->current_result_id_ << " this->result_id_: " << this->result_id_ << std::endl;
      }
      assert(connection_->current_result_id == this->result_id_);
      if (!current_result_)
      {
        current_result_nrows_ = 0;
        row_i_ = 0;
        end_of_result_ = true; 
        this->connection_->end_of_current_result(fiber_);
        // std::cout << " connection_->current_result_id " << connection_->current_result_id << " this->result_id_: " << this->result_id_ << std::endl;
        // assert(connection_->current_result_id != this->result_id_);
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

  if (end_of_result_) return false;
  // println("pgsql_result<B>::read");

  // Just for the first row, we fetch the result.
  // For the next calls fetch_next_result is called at the end of this function.
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

  this->fetch_next_result<T>(std::forward<T>(output));
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
