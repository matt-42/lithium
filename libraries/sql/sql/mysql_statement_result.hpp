#pragma once

#include <li/sql/internal/utils.hh>

namespace li {

template <typename B> long long int mysql_statement_result<B>::affected_rows() {
  return mysql_stmt_affected_rows(data_.stmt_);
}

template <typename B>
template <typename T>
void mysql_statement_result<B>::fetch_column(MYSQL_BIND* b, unsigned long, T&, int) {
  if (*b->error) {
    throw std::runtime_error("Result could not fit in the provided types: loss of sign or significant digits or type mismatch.");
  }
}

template <typename B>
void mysql_statement_result<B>::fetch_column(MYSQL_BIND* b, unsigned long real_length,
                                             std::string& v, int i) {
  // If the string was big enough to hold the result string, return it.
  if (real_length <= v.size()) {
    v.resize(real_length);
    return;
  }
  // Otherwise we need to call mysql_stmt_fetch_column again to get the result string.

  // Reserve enough space to fetch the string.
  v.resize(real_length);

  // Bind result.
  b[i].buffer_length = real_length;
  b[i].buffer = &v[0];
  mysql_stmt_bind_result(data_.stmt_, b);
  result_allocated_ = true;

  if (mysql_stmt_fetch_column(data_.stmt_, b + i, i, 0) != 0) {
    *connection_status_ = 1;
    throw std::runtime_error(std::string("mysql_stmt_fetch_column error: ") +
                             mysql_stmt_error(data_.stmt_));
  }
}

template <typename B>
template <unsigned SIZE>
void mysql_statement_result<B>::fetch_column(MYSQL_BIND& b, unsigned long real_length,
                                             sql_varchar<SIZE>& v, int i) {
  v.resize(real_length);
}

template <typename B> template <typename T> bool mysql_statement_result<B>::read(T&& output) {
  // Todo: skip useless mysql_bind_output and mysql_stmt_bind_result if too costly.
  // if (bound_ptr_ == (void*)output)...
  try {

    // Bind output.
    auto bind_data = mysql_bind_output(data_, std::forward<T>(output));
    unsigned long* real_lengths = bind_data.real_lengths.data();
    MYSQL_BIND* bind = bind_data.bind.data();

    bool bind_ret = mysql_stmt_bind_result(data_.stmt_, bind_data.bind.data());
    // std::cout << "bind_ret: " << bind_ret << std::endl;
    if (bind_ret != 0)
    {
      throw std::runtime_error(std::string("mysql_stmt_bind_result error: ") + mysql_stmt_error(data_.stmt_));
    }

    result_allocated_ = true;

    // Fetch row.
    // Note: This also advance to the next row.
    int res = mysql_wrapper_.mysql_stmt_fetch(connection_status_, data_.stmt_);
    if (res == MYSQL_NO_DATA) // If end of result, return false.
      return false;

    // Finalize fetch:
    //    - fetch strings that did not fit the preallocated strings.
    //    - check for truncated data errors.
    //    - resize preallocated strings that were bigger than the request result.
    if constexpr (is_tuple<T>::value) {
      int i = 0;
      tuple_map(std::forward<T>(output), [&](auto& m) {
        this->fetch_column(bind, real_lengths[i], m, i);
        i++;
      });
    } else {
      li::map(std::forward<T>(output), [&](auto k, auto& v) {
        for (int i = 0; i < data_.num_fields_; i++)
          if (!strcmp(data_.fields_[i].name, li::symbol_string(k)))
            this->fetch_column(bind, real_lengths[i], v, i);
      });
    }
    return true;
  } catch (const std::runtime_error& e) {
    mysql_stmt_reset(data_.stmt_);
    throw e;
  }

}

template <typename B> long long int mysql_statement_result<B>::last_insert_id() {
  return mysql_stmt_insert_id(data_.stmt_);
}

} // namespace li
