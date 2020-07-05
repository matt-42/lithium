#pragma once

namespace li {

template <typename Y> PGresult* pgsql_statement_result<Y>::wait_for_next_result() {
  // std::cout << "WAIT ======================" << std::endl;
  while (true) {
    if (PQconsumeInput(connection_) == 0)
      throw std::runtime_error(std::string("PQconsumeInput() failed: ") +
                               PQerrorMessage(connection_));

    if (PQisBusy(connection_)) {
      // std::cout << "isbusy" << std::endl;
      try {
        fiber_.yield();
      } catch (typename Y::exception_type& e) {
        // Yield thrown a exception (probably because a closed connection).
        // Mark the connection as broken because it is left in a undefined state.
        *connection_status_ = 1;
        throw std::move(e);
      }
    } else {
      // std::cout << "notbusy" << std::endl;
      PGresult* res = PQgetResult(connection_);
      if (PQresultStatus(res) == PGRES_FATAL_ERROR and PQerrorMessage(connection_)[0] != 0)
        throw std::runtime_error(std::string("Postresql fatal error:") +
                                 PQerrorMessage(connection_));
      else if (PQresultStatus(res) == PGRES_NONFATAL_ERROR)
        std::cerr << "Postgresql non fatal error: " << PQerrorMessage(connection_) << std::endl;
      return res;
    }
  }
}

template <typename Y> void pgsql_statement_result<Y>::flush_results() {
  while (PGresult* res = wait_for_next_result())
    PQclear(res);
}

// Fetch a string from a result field.
template <typename Y>
template <typename... A>
void pgsql_statement_result<Y>::fetch_value(std::string& out, char* val, int length,
                                            bool is_binary) {
  // assert(!is_binary);
  // std::cout << "fetch string: " << length << " '"<< val <<"'" << std::endl;
  out = std::move(std::string(val, strlen(val)));
}

// Fetch a blob from a result field.
template <typename Y>
template <typename... A>
void fetch_value(sql_blob& out, char* val, int length, bool is_binary) {
  // assert(is_binary);
  out = std::move(std::string(val, length));
}

// Fetch an int from a result field.
template <typename Y> void fetch_value(int& out, char* val, int length, bool is_binary) {
  assert(is_binary);
  // std::cout << "fetch integer " << length << " " << is_binary << std::endl;
  // std::cout << "fetch integer " << be64toh(*((uint64_t *) val)) << std::endl;
  if (length == 8) {
    // std::cout << "fetch 64b integer " << std::hex << int(32) << std::endl;
    // std::cout << "fetch 64b integer " << std::hex << uint64_t(*((uint64_t *) val)) << std::endl;
    // std::cout << "fetch 64b integer " << std::hex << (*((uint64_t *) val)) << std::endl;
    // std::cout << "fetch 64b integer " << std::hex << be64toh(*((uint64_t *) val)) << std::endl;
    out = be64toh(*((uint64_t*)val));
  } else if (length == 4)
    out = (uint32_t)ntohl(*((uint32_t*)val));
  else if (length == 2)
    out = (uint16_t)ntohs(*((uint16_t*)val));
  else
    assert(0);
}

// Fetch an unsigned int from a result field.
template <typename Y>
void pgsql_statement_result<Y>::fetch_value(unsigned int& out, char* val, int length,
                                            bool is_binary) {
  assert(is_binary);
  if (length == 8)
    out = be64toh(*((uint64_t*)val));
  else if (length == 4)
    out = ntohl(*((uint32_t*)val));
  else if (length == 2)
    out = ntohs(*((uint16_t*)val));
  else
    assert(0);
}

template <typename B> template <typename T> bool pgsql_statement_result<B>::read(T&& output) {

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
  }

  // Tuples
  if constexpr (is_tuple<T>::value) {
    int field_i = 0;

    int nfields = PQnfields(res);
    if (nfields != sizeof...(A))
      throw std::runtime_error("postgresql error: in fetch: Mismatch between the request number of "
                               "field and the outputs.");

    tuple_map(std::forward<T>(output), [&](auto& m) {
      fetch_value(m, PQgetvalue(res, row_i, field_i), PQgetlength(res, row_i, field_i),
                  PQfformat(res, field_i));
      field_i++;
    });
  } else { // Metamaps.
    li::map(std::forward<T>(output), [&](auto k, auto& m) {
      int field_i = PQfnumber(res, symbol_string(k));
      if (field_i == -1)
        throw std::runtime_error(std::string("postgresql errror : Field ") + symbol_string(k) +
                                 " not fount in result.");

      fetch_value(m, PQgetvalue(res, row_i, field_i), PQgetlength(res, row_i, field_i),
                  PQfformat(res, field_i));
    });
  }

  this->row_i_++;

  return true;
}

// Get the last id of the row inserted by the last command.
template <typename Y> long long int pgsql_statement_result<Y>::last_insert_id() {
  // while (PGresult* res = wait_for_next_result())
  //  PQclear(res);
  // PQsendQuery(connection_, "LASTVAL()");
  return this->read<int>();
  // PGresult *PQexec(connection_, const char *command);
  // this->operator()
  //   last_insert_id_ = PQoidValue(res);
  //   std::cout << "id " << last_insert_id_ << std::endl;
  //   PQclear(res);
  // }
  // return last_insert_id_;
}

} // namespace li