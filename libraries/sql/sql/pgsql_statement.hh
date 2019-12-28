#pragma once

#include <libpq-fe.h>
#include <arpa/inet.h>

#include <li/callable_traits/callable_traits.hh>
#include <li/metamap/metamap.hh>
#include <li/sql/sql_common.hh>
#include <li/sql/symbols.hh>
#include <memory>

namespace li {


struct pgsql_statement_data : std::enable_shared_from_this<pgsql_statement_data> {
  pgsql_statement_data(const std::string& s) : stmt_name(s) {}
  std::string stmt_name;
};

template <typename Y>
struct pgsql_statement {

  // Wait for the next result.
  PGresult* wait_for_next_result() {
    //std::cout << "WAIT ======================" << std::endl;
    while (true)
    {
      if (PQconsumeInput(connection_) == 0)
        throw std::runtime_error(std::string("PQconsumeInput() failed: ") + PQerrorMessage(connection_));

      if (PQisBusy(connection_))
      {
        //std::cout << "isbusy" << std::endl;
        try {
          yield_();
        } catch (typename Y::exception_type& e) {
          // Yield thrown a exception (probably because a closed connection).
          // Mark the connection as broken because it is left in a undefined state.
          *connection_status_ = 1;
          throw std::move(e);
        }
      }
      else 
      {
        //std::cout << "notbusy" << std::endl;
        PGresult* res =  PQgetResult(connection_);
        if (PQresultStatus(res) == PGRES_FATAL_ERROR and PQerrorMessage(connection_)[0] != 0)
          throw std::runtime_error(std::string("Postresql fatal error:") + PQerrorMessage(connection_));
        else if (PQresultStatus(res) == PGRES_NONFATAL_ERROR)
          std::cerr << "Postgresql non fatal error: " << PQerrorMessage(connection_) << std::endl;
        return res;
      }
    }
  }

  void flush_results()
  {
    while (PGresult* res = wait_for_next_result())
      PQclear(res);
  }
  
  // Execute a simple request takes no arguments and return no rows.
  auto& operator()() {

    //std::cout << "sending " << data_.stmt_name.c_str() << std::endl;
    flush_results();
    if (!PQsendQueryPrepared(connection_, data_.stmt_name.c_str(), 0, nullptr, nullptr, nullptr, 1))
      throw std::runtime_error(std::string("Postresql error:") + PQerrorMessage(connection_));
    
    return *this;
  }


  // Execute a request with placeholders.
  template <unsigned N>
  void bind_param(sql_varchar<N>&& m, const char*& values, int& lengths, int& binary)
  {
    //std::cout << "send param varchar " << m << std::endl;
    values = m.c_str(); lengths = m.size(); binary = 0;
  }
  template <unsigned N>
  void bind_param(const sql_varchar<N>&& m, const char*& values, int& lengths, int& binary)
  {
    //std::cout << "send param const varchar " << m << std::endl;
    values = m.c_str(); lengths = m.size(); binary = 0;
  }
  void bind_param(const char* m, const char*& values, int& lengths, int& binary)
  {
    //std::cout << "send param const char*[N] " << m << std::endl;
    values = m; lengths = strlen(m); binary = 0;
  }

  template <typename... T> auto& operator()(T&&... args) {

    constexpr int nparams = sizeof...(T); 
    const char* values[nparams];
    int lengths[nparams];;
    int binary[nparams];

    int i = 0;
    tuple_map(std::forward_as_tuple(std::forward<T>(args)...), [&](auto&& m) {
      if constexpr(std::is_same<std::decay_t<decltype(m)>, std::string>::value or
                   std::is_same<std::decay_t<decltype(m)>, std::string_view>::value)
      {
        //std::cout << "send param string: " << m << std::endl;
        values[i] = m.c_str();
        lengths[i] = m.size();
        binary[i] = 0;
      }
      else if constexpr(std::is_same<std::remove_reference_t<decltype(m)>, const char*>::value)
      {
        //std::cout << "send param const char* " << m << std::endl;
        values[i] = m;
        lengths[i] = strlen(m);
        binary[i] = 0;
      }
      else if constexpr(std::is_same<std::decay_t<decltype(m)>, int>::value)
      {
        values[i] = (char*)new int(htonl(m));
        lengths[i] = sizeof(m);
        binary[i] = 1;
      }
      else if constexpr(std::is_same<std::decay_t<decltype(m)>, long long int>::value)
      {
        // FIXME send 64bit values.
        //std::cout << "long long int param: " << m << std::endl;
        values[i] = (char*)new int(htonl(uint32_t(m)));
        lengths[i] = sizeof(uint32_t);
        // does not work:
        //values[i] = (char*)new uint64_t(htobe64((uint64_t) m));
        //lengths[i] = sizeof(uint64_t);
        binary[i] = 1;
      }
      else 
      {
        bind_param(std::move(m), values[i], lengths[i], binary[i]);
      }
      // Fixme other types.
      i++;
    });

    flush_results();
    //std::cout << "sending " << data_.stmt_name.c_str() << " with " << nparams << " params" << std::endl;
    if (!PQsendQueryPrepared(connection_, data_.stmt_name.c_str(), nparams, values, lengths, binary, 1))
      throw std::runtime_error(std::string("Postresql error:") + PQerrorMessage(connection_));
    
    return *this;
  }

  //FIXME long long int affected_rows() { return pgsql_stmt_affected_rows(data_.stmt_); }

  // Fetch a string from a result field.
  template <typename... A> void fetch_impl(std::string& out, char* val, int length, bool is_binary) {
    //assert(!is_binary);
    //std::cout << "fetch string: " << length << " '"<< val <<"'" << std::endl;
    out = std::move(std::string(val, strlen(val)));
  }

  // Fetch a blob from a result field.
  template <typename... A> void fetch_impl(sql_blob& out, char* val, int length, bool is_binary) {
    //assert(is_binary);
    out = std::move(std::string(val, length));
  }

  // Fetch an int from a result field.
  void fetch_impl(int& out, char* val, int length, bool is_binary) {
    assert(is_binary);
    //std::cout << "fetch integer " << length << " " << is_binary << std::endl;
    // std::cout << "fetch integer " << be64toh(*((uint64_t *) val)) << std::endl;
    if (length == 8) 
    {
      // std::cout << "fetch 64b integer " << std::hex << int(32) << std::endl;
      // std::cout << "fetch 64b integer " << std::hex << uint64_t(*((uint64_t *) val)) << std::endl;
      // std::cout << "fetch 64b integer " << std::hex << (*((uint64_t *) val)) << std::endl;
      // std::cout << "fetch 64b integer " << std::hex << be64toh(*((uint64_t *) val)) << std::endl;
      out = be64toh(*((uint64_t *) val));
    }
    else if (length == 4) out = (uint32_t) ntohl(*((uint32_t *) val));
    else if (length == 2) out = (uint16_t) ntohs(*((uint16_t *) val));
    else assert(0);
  }

  // Fetch an unsigned int from a result field.
  void fetch_impl(unsigned int& out, char* val, int length, bool is_binary) {
    assert(is_binary);
    if (length == 8) out = be64toh(*((uint64_t *) val));
    else if (length == 4) out = ntohl(*((uint32_t *) val));
    else if (length == 2) out = ntohs(*((uint16_t *) val));
    else assert(0);
  }

  // Fetch a complete row in a metamap.
  template <typename... A> void fetch(PGresult* res, int row_i, metamap<A...>& o) {
    li::map(o, [&] (auto k, auto& m) {
      int field_i = PQfnumber(res, symbol_string(k));
      if (field_i == -1)
        throw std::runtime_error(std::string("postgresql errror : Field ") + symbol_string(k) + " not fount in result."); 

      fetch_impl(m, PQgetvalue(res, row_i, field_i),
                 PQgetlength(res, row_i, field_i), 
                 PQfformat(res, field_i));
    });
  }

  // Fetch a complete row in a tuple.
  template <typename... A> void fetch(PGresult* res, int row_i, std::tuple<A...>& o) {
    int field_i = 0;

    int nfields = PQnfields(res);
    if (nfields != sizeof...(A))
      throw std::runtime_error("postgresql error: in fetch: Mismatch between the request number of field and the outputs.");

    tuple_map(o, [&] (auto& m) {
      fetch_impl(m, PQgetvalue(res, row_i, field_i),
                 PQgetlength(res, row_i, field_i), 
                 PQfformat(res, field_i));
      field_i++;
    });
  }
  
  template <typename T> void fetch(PGresult* res, int row_i, T& o) {
    int field_i = 0;

    int nfields = PQnfields(res);
    if (nfields != 1)
      throw std::runtime_error("postgresql error: in fetch: Mismatch between the request number of field and the outputs.");
    fetch_impl(o, PQgetvalue(res, row_i, 0),
                 PQgetlength(res, row_i, 0), 
                 PQfformat(res, 0));
  }

  // Fetch one object (expect a resultset of maximum 1 row).
  // Return 0 if no rows, 1 if the fetch is ok.
  template <typename T> int fetch_one(T& o) {
    PGresult* res = wait_for_next_result();


    int nrows = PQntuples(res);
    if (nrows == 0)
      return 0;
    if (nrows > 1)
      throw std::runtime_error("postgresql error: in fetch_one: The request returned more than one row.");

    fetch(res, 0, o);
    PQclear(res);
    return 1;
  }

  // Read a single optional value.
  template <typename T> void read(std::optional<T>& o) {
    T t;
    if (fetch_one(t))
      o = std::optional<T>(t);
    else
      o = std::optional<T>();
  }

  // Read a single value.
  template <typename T> T read() {
    T t;
    if (!fetch_one(t))
      throw std::runtime_error("Request did not return any data.");
    return t;
  }

  // Read a single optional value.
  template <typename T> std::optional<T> read_optional() {
    T t;
    if (fetch_one(t))
      return std::optional<T>(t);
    else
      return std::optional<T>();
  }

  template <typename T>
  struct unconstref_tuple_elements {};
  template <typename... T>
  struct unconstref_tuple_elements<std::tuple<T...>> {
    typedef std::tuple<std::remove_const_t<std::remove_reference_t<T>>...> ret;
  };
  
  // Map a function to multiple rows.
  template <typename F> void map(F f) {
    typedef typename unconstref_tuple_elements<callable_arguments_tuple_t<F>>::ret tp;
    typedef std::remove_const_t<std::remove_reference_t<std::tuple_element_t<0, tp>>> T;

    while(PGresult* res = wait_for_next_result())
    {
      int nrows = PQntuples(res);
      for (int row_i = 0; row_i < nrows; row_i++)
      {
        if constexpr (li::is_metamap<T>::ret) {
          T o;
          fetch(res, row_i, o);
          f(o);
        } else { // tuple version.
          tp o;
          fetch(res, row_i, o);
          std::apply(f, o);
        }
      }
      PQclear(res);
    }

  }

  // Get the last id of the row inserted by the last command.
  long long int last_insert_id() { 
    //while (PGresult* res = wait_for_next_result())
    //  PQclear(res);
    //PQsendQuery(connection_, "LASTVAL()");
    return this->read<int>();
    // PGresult *PQexec(connection_, const char *command);
    // this->operator()
    //   last_insert_id_ = PQoidValue(res);
    //   std::cout << "id " << last_insert_id_ << std::endl;
    //   PQclear(res);
    // }
    // return last_insert_id_; 
  }

  // Return true if the request returns an empty set.  
  bool empty() {
    PGresult* res = wait_for_next_result();
    int nrows = PQntuples(res);
    PQclear(res);
    return nrows = 0;
  }

  void wait () {
    while (PGresult* res = wait_for_next_result())
      PQclear(res);
  }

  PGconn* connection_;
  Y& yield_;
  pgsql_statement_data& data_;
  std::shared_ptr<int> connection_status_;
  int last_insert_id_ = -1;
};

}
