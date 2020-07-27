#pragma once

#include <li/sql/type_hashmap.hh>

namespace li
{

struct pgsql_statement_data;
struct pgsql_connection_data {

  ~pgsql_connection_data() {
    if (pgconn_) {
      cancel();
      PQfinish(pgconn_);
    }
  }
  void cancel() {
    if (pgconn_) {
      // Cancel any pending request.
      PGcancel* cancel = PQgetCancel(pgconn_);
      char x[256];
      if (cancel) {
        PQcancel(cancel, x, 256);
        PQfreeCancel(cancel);
      }
    }
  }
  template <typename Y>
  void flush(Y& fiber) {
    while(int ret = PQflush(pgconn_))
    {
      if (ret == -1)
      {
        std::cerr << "PQflush error" << std::endl;
      }
      if (ret == 1)
        fiber.yield();
    }
  }

  PGconn* pgconn_ = nullptr;
  int fd = -1;
  std::unordered_map<std::string, std::shared_ptr<pgsql_statement_data>> statements;
  type_hashmap<std::shared_ptr<pgsql_statement_data>> statements_hashmap;
  int error_ = 0;
};

}