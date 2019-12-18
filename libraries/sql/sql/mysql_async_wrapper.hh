#pragma once

#include <mysql.h>

#include <li/callable_traits/callable_traits.hh>
#include <li/metamap/metamap.hh>
#include <li/sql/sql_common.hh>
#include <li/sql/symbols.hh>

namespace li {

// Blocking version.
struct mysql_functions_blocking {
  enum { is_blocking = true };

#define LI_MYSQL_BLOCKING_WRAPPER(FN)                                                              \
  template <typename... A> auto FN(std::shared_ptr<int> connection_status, A&&... a) {\
    int ret = ::FN(std::forward<A>(a)...); \
    if (ret)\
      *connection_status = 1;\
    return ret; }

  LI_MYSQL_BLOCKING_WRAPPER(mysql_fetch_row)
  LI_MYSQL_BLOCKING_WRAPPER(mysql_real_query)
  LI_MYSQL_BLOCKING_WRAPPER(mysql_stmt_execute)
  LI_MYSQL_BLOCKING_WRAPPER(mysql_stmt_reset)
  LI_MYSQL_BLOCKING_WRAPPER(mysql_stmt_prepare)
  LI_MYSQL_BLOCKING_WRAPPER(mysql_stmt_fetch)
  LI_MYSQL_BLOCKING_WRAPPER(mysql_stmt_free_result)

#undef LI_MYSQL_BLOCKING_WRAPPER
};

// Non blocking version.
template <typename Y> struct mysql_functions_non_blocking {
  enum { is_blocking = false };

  template <typename RT, typename A1, typename... A, typename B1, typename... B>
  auto mysql_non_blocking_call(std::shared_ptr<int> connection_status, int fn_start(RT*, B1, B...),
                               int fn_cont(RT*, B1, int), A1&& a1, A&&... args) {

    RT ret;
    int status = fn_start(&ret, std::forward<A1>(a1), std::forward<A>(args)...);

    bool error = false;
    while (status) {
      try {
        yield_();
      } catch (typename Y::exception_type& e) {
        // Yield thrown a exception (probably because a closed connection).
        // Mark the connection as broken because it is left in a undefined state.
        *connection_status = 1;
        throw std::move(e);
      }

      status = fn_cont(&ret, std::forward<A1>(a1), status);
    }
    if (ret)
    {
      *connection_status = 1;
    }
    return ret;
  }

#define LI_MYSQL_NONBLOCKING_WRAPPER(FN)                                                           \
  template <typename... A> auto FN(std::shared_ptr<int> connection_status, A&&... a) {                                                     \
    return mysql_non_blocking_call(connection_status, ::FN##_start, ::FN##_cont, std::forward<A>(a)...);              \
  }

  LI_MYSQL_NONBLOCKING_WRAPPER(mysql_fetch_row)
  LI_MYSQL_NONBLOCKING_WRAPPER(mysql_real_query)
  LI_MYSQL_NONBLOCKING_WRAPPER(mysql_stmt_execute)
  LI_MYSQL_NONBLOCKING_WRAPPER(mysql_stmt_reset)
  LI_MYSQL_NONBLOCKING_WRAPPER(mysql_stmt_prepare)
  LI_MYSQL_NONBLOCKING_WRAPPER(mysql_stmt_fetch)
  LI_MYSQL_NONBLOCKING_WRAPPER(mysql_stmt_free_result)

#undef LI_MYSQL_NONBLOCKING_WRAPPER

  Y yield_;
};

} // namespace li
