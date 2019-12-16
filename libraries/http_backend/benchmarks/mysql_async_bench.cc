#include <li/http_backend/http_backend.hh>
#include <li/http_client/http_client.hh>
#include <li/sql/mysql.hh>

#include "symbols.hh"
#include "client.hh"

using namespace li;

thread_local std::deque<MYSQL*> async_pool_;
thread_local std::deque<MYSQL*> sync_pool_;
thread_local std::deque<std::pair<MYSQL*, MYSQL_STMT*>> async_pool_stmt;
thread_local std::deque<std::pair<MYSQL*, MYSQL_STMT*>> sync_pool_stmt;

template <typename C>
struct my_yield
{
  void operator()() { ctx.write(nullptr, 0); }
  void listen_to_fd(int fd) { ctx.listen_to_new_fd(fd); }
  C& ctx;
};

struct my_yield2
{
  void operator()() {  }
  void listen_to_fd(int fd) { }
};

int main(int argc, char* argv[]) {

  auto mysql_db =
      mysql_database(s::host = "127.0.0.1", s::database = "silicon_test", s::user = "root",
                     s::password = "sl_test_password", s::port = 14550, s::charset = "utf8");
  //mysql_db.connect();
  //std::cout << mysql_db.connect()("SELECT 1+2;").read<int>() << std::endl;;
  //auto c = mysql_db.async_connect(my_yield2{});
  //auto c = mysql_db.connect();
  //std::cout << c("SELECT 1+2;").read<int>() << std::endl;
  //return 0;    
  http_api my_api;

  my_api.get("/json") = [&](http_request& request, http_response& response) {
    response.write_json(s::message = "Hello world!");
  };
  my_api.get("/plaintext") = [&](http_request& request, http_response& response) {
    response.write("Hello world!");
  };
  my_api.get("/select") = [&](http_request& request, http_response& response) {
    auto c = mysql_db.connect();
    response.write_json(s::message = c("SELECT 1+2;").read<int>());
  };

  my_api.get("/new_async_api") = [&](http_request& request, http_response& response) {
    auto c = mysql_db.connect(my_yield<decltype(request.http_ctx)>{request.http_ctx});
    //auto c = mysql_db.async_connect(my_yield2{});
    response.write_json(s::message = c("SELECT 1+2;").read<int>());
  };

  my_api.get("/async") = [&](http_request& request, http_response& response) {
    auto yield = [&]() { 
      request.http_ctx.write(nullptr, 0); 
    };

    MYSQL* mysql = nullptr;
    if (!async_pool_.empty()) {
      mysql = async_pool_.back();
      async_pool_.pop_back();
    }

    if (!mysql) {
      mysql = new MYSQL;
      mysql_init(mysql);
      //std::cout << "mysql_init" << std::endl;
      mysql_options(mysql, MYSQL_OPT_NONBLOCK, 0);
      MYSQL* connection = nullptr;
      int status = mysql_real_connect_start(&connection, mysql, "127.0.0.1", "root",
                                            "sl_test_password", "silicon_test", 14550, NULL, 0);
      //std::cout << "initialized" << std::endl;
      //std::cout << "socket: " << mysql_get_socket(mysql) << std::endl;
      request.http_ctx.listen_to_new_fd(mysql_get_socket(mysql));
      while (status) {
        yield();
        status = mysql_real_connect_cont(&connection, mysql, status);
      }
      if (!connection)
        std::cout << "Failed to mysql_real_connect()" << std::endl;
    } else {
      request.http_ctx.listen_to_new_fd(mysql_get_socket(mysql));
    }

    int status;

    int err;
    status = mysql_real_query_start(&err, mysql, "SELECT 1+2;", strlen("SELECT 1+2;"));
    while (status) {
      yield();
      status = mysql_real_query_cont(&err, mysql, status);
    }
    if (err)
      std::cout << "mysql_real_query() returns error" << std::endl;

    /* This method cannot block. */
    MYSQL_RES* res;
    MYSQL_ROW row;
    res = mysql_use_result(mysql);
    if (!res)
      std::cout << "mysql_use_result() returns error" << std::endl;

    for (;;) {
      //mysql_async_call(yield, mysql_fetch_row_start, mysql_fetch_row_cont, &row, res);
      status = mysql_fetch_row_start(&row, res);
      while (status) {
        // status= wait_for_mysql(mysql, status);
        yield();
        status = mysql_fetch_row_cont(&row, res, status);
      }
      if (!row)
        break;
      // printf("%s: %s\n", row[0], row[1]);
      response.write(row[0]);
      break;
    }
    if (mysql_errno(mysql))
      std::cout << "Got error while retrieving rows" << std::endl;

    mysql_free_result(res);
    async_pool_.push_back(mysql);
    // mysql_close(mysql);
    // std::cout << "done" << std::endl;
  };

  my_api.get("/sync") = [&](http_request& request, http_response& response) {
    auto yield = [&]() { request.http_ctx.write(nullptr, 0); };

    MYSQL* mysql = nullptr;
    if (!sync_pool_.empty()) {
      mysql = sync_pool_.back();
      sync_pool_.pop_back();
    }

    if (!mysql) {
      mysql = new MYSQL;
      mysql_init(mysql);
      MYSQL* connection = nullptr;
      mysql = mysql_real_connect(mysql, "127.0.0.1", "root",
                                            "sl_test_password", "silicon_test", 14550, NULL, 0);
    }

    int status;

    int err;
    status = mysql_real_query(mysql, "SELECT 1+2;", strlen("SELECT 1+2;"));
    
    /* This method cannot block. */
    MYSQL_RES* res;
    MYSQL_ROW row;
    res = mysql_use_result(mysql);
    if (!res)
      std::cout << "mysql_use_result() returns error" << std::endl;

    for (;;) {
      row = mysql_fetch_row(res);
      if (!row)
        break;
      // printf("%s: %s\n", row[0], row[1]);
      response.write(row[0]);
      break;
    }
    if (mysql_errno(mysql))
      std::cout << "Got error while retrieving rows" << std::endl;

    mysql_free_result(res);
    sync_pool_.push_back(mysql);
    // mysql_close(mysql);
    // std::cout << "done" << std::endl;
  };


  my_api.get("/async_prepare") = [&](http_request& request, http_response& response) {
  
    int final_result = 0;

    //std::cout << "START async_pool_stmt.size(): " << async_pool_stmt.size() << std::endl;
    auto yield = [&]() { request.http_ctx.write(nullptr, 0); };

    MYSQL* mysql = nullptr;
    MYSQL_STMT* stmt = nullptr;
    int status;
    int err;

    if (!async_pool_stmt.empty()) {
      //std::cout << "reuse" << std::endl;
      std::tie(mysql, stmt) = async_pool_stmt.back();
      async_pool_stmt.pop_back();
      request.http_ctx.listen_to_new_fd(mysql_get_socket(mysql));

    } else {
      mysql = new MYSQL;
      mysql_init(mysql);
      mysql_options(mysql, MYSQL_OPT_NONBLOCK, 0);
      MYSQL* connection = nullptr;
      int status = mysql_real_connect_start(&connection, mysql, "127.0.0.1", "root",
                                            "sl_test_password", "silicon_test", 14550, NULL, 0);
      request.http_ctx.listen_to_new_fd(mysql_get_socket(mysql));
      while (status) {
        yield();
        status = mysql_real_connect_cont(&connection, mysql, status);
      }
      if (!connection)
        std::cout << "Failed to mysql_real_connect()" << std::endl;

      // prepare
      stmt = mysql_stmt_init(mysql);
      if (!stmt)
        throw std::runtime_error(std::string("mysql_stmt_init error: ") + mysql_error(mysql));

      status = mysql_stmt_prepare_start(&err, stmt, "SELECT 1+2;", strlen("SELECT 1+2;"));
      while (status) {
        yield();
        status = mysql_stmt_prepare_cont(&err, stmt, status);
      }
      if (err)
        std::cout << "mysql_stmt_prepare_cont() returns error" << std::endl;
    }

    final_result = -1;
    // execute
    status = mysql_stmt_execute_start(&err, stmt);
    while (status) {
      yield();
      status = mysql_stmt_execute_cont(&err, stmt, status);
    }
    if (err)
      std::cout << "mysql_stmt_execute_cont() returns error" << std::endl;

    // bind.
    MYSQL_BIND bind;
    memset(&bind, 0, sizeof(bind));
    bind.buffer = &final_result;
    bind.buffer_type = MYSQL_TYPE_LONG;
    bind.is_unsigned = false;
    mysql_stmt_bind_result(stmt, &bind);

    // FETCH
    status = mysql_stmt_fetch_start(&err, stmt);
    while (status) {
      yield();
      status = mysql_stmt_fetch_cont(&err, stmt, status);
    }
    if (err)
      std::cout << "mysql_stmt_fetch_cont() returns error" << std::endl;
    
    if (mysql_errno(mysql))
      std::cout << "Got error while retrieving rows" << std::endl;

    my_bool my_bool = 42;
    mysql_stmt_free_result_start(&my_bool, stmt);
    while (status) {
      yield();
      status = mysql_stmt_free_result_cont(&my_bool, stmt, status);
    }
      async_pool_stmt.push_back({mysql, stmt});
    response.write_json(s::age = final_result);
  };

  my_api.get("/sync_prepare") = [&](http_request& request, http_response& response) {
  
    int final_result = 0;

    MYSQL* mysql = nullptr;
    MYSQL_STMT* stmt = nullptr;
    int status;
    int err;

    if (!sync_pool_stmt.empty()) {
      //std::cout << "reuse" << std::endl;
      std::tie(mysql, stmt) = sync_pool_stmt.back();
      sync_pool_stmt.pop_back();

    } else {
      mysql = new MYSQL;
      mysql_init(mysql);
      MYSQL* connection = nullptr;
      mysql_real_connect(mysql, "127.0.0.1", "root",
                                            "sl_test_password", "silicon_test", 14550, NULL, 0);

      // prepare
      stmt = mysql_stmt_init(mysql);
      if (!stmt)
        throw std::runtime_error(std::string("mysql_stmt_init error: ") + mysql_error(mysql));

      err = mysql_stmt_prepare(stmt, "SELECT 1+2;", strlen("SELECT 1+2;"));
      if (err)
        std::cout << "mysql_stmt_prepare_cont() returns error" << std::endl;
    }

    final_result = -1;
    // execute
    err = mysql_stmt_execute(stmt);

    // bind.
    MYSQL_BIND bind;
    memset(&bind, 0, sizeof(bind));
    bind.buffer = &final_result;
    bind.buffer_type = MYSQL_TYPE_LONG;
    bind.is_unsigned = false;
    mysql_stmt_bind_result(stmt, &bind);

    // FETCH
    err = mysql_stmt_fetch(stmt);
    if (err)
      std::cout << "mysql_stmt_fetch_cont() returns error" << std::endl;
    
    if (mysql_errno(mysql))
      std::cout << "Got error while retrieving rows" << std::endl;

    my_bool my_bool = 42;
    mysql_stmt_free_result(stmt);
    sync_pool_stmt.push_back({mysql, stmt});
    response.write_json(s::age = final_result);
  };

  int port = 12360;
  http_serve(my_api, port); return 0;
  
  //http_serve(my_api, port, s::non_blocking);
  //benchmark(1, 1, 25, port, "GET /json HTTP/1.1\r\nHost: localhost\r\n\r\n");
}
