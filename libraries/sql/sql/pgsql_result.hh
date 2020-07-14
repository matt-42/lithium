#pragma once

namespace li {

template <typename Y> struct pgsql_result {

public:
  ~pgsql_result() { if (current_result_) PQclear(current_result_); }
  // Read metamap and tuples.
  template <typename T> bool read(T&& t1);
  long long int last_insert_id();
  // Flush all results.
  void flush_results();

  PGconn* connection_;
  Y& fiber_;
  std::shared_ptr<int> connection_status_;

  int last_insert_id_ = -1;
  int row_i_ = 0;
  int current_result_nrows_ = 0;
  PGresult* current_result_ = nullptr;
  std::vector<Oid> curent_result_field_types_;
  std::vector<int> curent_result_field_positions_;
  
private:

  // Wait for the next result.
  PGresult* wait_for_next_result();

  // Fetch a string from a result field.
  template <typename... A>
  void fetch_value(std::string& out, char* val, int length, bool is_binary, Oid field_type);
  // Fetch a blob from a result field.
  template <typename... A> void fetch_value(sql_blob& out, char* val, int length, bool is_binary, Oid field_type);
  // Fetch an int from a result field.
  void fetch_value(int& out, char* val, int length, bool is_binary, Oid field_type);
  // Fetch an unsigned int from a result field.
  void fetch_value(unsigned int& out, char* val, int length, bool is_binary, Oid field_type);
};

} // namespace li
#include <li/sql/pgsql_result.hpp>
