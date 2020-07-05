#pragma once

namespace li {

template <typename B> mysql_statement_result<B> mysql_statement<B>::operator()() {
  // Call the statement without arguments.
  mysql_wrapper_.mysql_stmt_execute(connection_status_, data_.stmt_);
  return mysql_statement_result<B>{mysql_wrapper_, data_, connection_status_};
}

template <typename B> template <typename... T>
sql_result<mysql_statement_result<B>> mysql_statement<B>::operator()(T&&... args) {


  // Bind the ...args in the MYSQL BIND structure.
  MYSQL_BIND bind[sizeof...(T)];
  memset(bind, 0, sizeof(bind));
  int i = 0;
  tuple_map(std::forward_as_tuple(args...), [&](auto& m) {
    mysql_bind_param(bind[i], m);
    i++;
  });

  // Pass MYSQL BIND to mysql.
  if (mysql_stmt_bind_param(data_.stmt_, bind) != 0) {
    *connection_status_ = 1;
    throw std::runtime_error(std::string("mysql_stmt_bind_param error: ") +
                             mysql_stmt_error(data_.stmt_));
  }

  // Execute the statement.
  mysql_wrapper_.mysql_stmt_execute(connection_status_, data_.stmt_);

  // Return the wrapped mysql result.
  return sql_result<mysql_statement_result<B>>{mysql_statement_result<B>{mysql_wrapper_, data_, connection_status_}};
}

} // namespace li
