#pragma once

#include <li/sql/sql_common.hh>
#include <li/sql/pgsql_connection_data.hh>

namespace li {


template <typename Y> struct pgsql_result {

public:
  pgsql_result(std::shared_ptr<pgsql_connection_data> connection_,
  Y& fiber_,
  int result_id_) : connection_(connection_), fiber_(fiber_), result_id_(result_id_) {
      // std::cout << " build result " << result_id_ << std::endl;

    cleaner_ = std::shared_ptr<int>((int*)42,
     [connection_, result_id_, this] (int* p) mutable {
       this->cleanup();
      //delete p;
      // std::cout << " destroy result " << std::endl;
      // std::cout << " destroy result " << result_id_ << " this: " << this->result_id_ << std::endl;
      if (this->end_of_result_) return;

      else if (connection_->current_result_id == result_id_)
      {
        connection_->flush_current_query_result();
        connection_->end_of_current_result(this->fiber_, false);
      }
      else {
        connection_->ignore_result(result_id_);
      }
    });

  }
  ~pgsql_result() {  }

  void cleanup() { if (current_result_) { PQclear(current_result_); current_result_ = nullptr; } }

  // Read metamap and tuples.
  template <typename T> bool read(T&& t1);
  long long int last_insert_id();
  
  // Flush all results.
  void flush_results();

  // Fetch next result, used internally by read.
  template <typename T> bool fetch_next_result(T&& output);

  std::shared_ptr<pgsql_connection_data> connection_;
  Y& fiber_;
  int result_id_;

  bool end_of_result_ = false;
  int last_insert_id_ = -1;
  int row_i_ = 0;
  int current_result_nrows_ = 0;
  PGresult* current_result_ = nullptr;
  std::vector<Oid> curent_result_field_types_;
  std::vector<int> curent_result_field_positions_;

  std::shared_ptr<int> cleaner_;

private:

  // Wait for the next result.
  PGresult* wait_for_next_result();

  // Fetch a string from a result field.
  template <typename... A>
  void fetch_value(std::string& out, int field_i, Oid field_type);
  // Fetch a blob from a result field.
  template <typename... A> void fetch_value(sql_blob& out, int field_i, Oid field_type);
  // Fetch an int from a result field.
  void fetch_value(int& out, int field_i, Oid field_type);
  // Fetch an unsigned int from a result field.
  void fetch_value(unsigned int& out, int field_i, Oid field_type);
};

} // namespace li

#include <li/sql/pgsql_result.hpp>
