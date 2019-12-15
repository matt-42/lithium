#include <li/http_backend/http_backend.hh>
#include <li/http_client/http_client.hh>
#include <li/sql/mysql.hh>

#include "symbols.hh"

using namespace li;

thread_local std::deque<MYSQL*> async_pool_;

int main(int argc, char* argv[]) {

  auto mysql_db =
      mysql_database(s::host = "127.0.0.1", s::database = "silicon_test", s::user = "root",
                     s::password = "sl_test_password", s::port = 14550, s::charset = "utf8");

  //auto c = mysql_db.connect();

  // c("DROP table if exists users_test_mysql;");
  // c("CREATE TABLE users_test_mysql (id int,name varchar(255),age int);");

  http_api my_api;

  my_api.get("/json") = [&](http_request& request, http_response& response) {
    response.write_json(s::message = "Hello world!");
  };
  my_api.get("/select") = [&](http_request& request, http_response& response) {
    auto c = mysql_db.connect();
    response.write_json(s::message = c("SELECT 1+2;").read<int>());
  };


  my_api.get("/async") = [&](http_request& request, http_response& response) {

    auto yield = [&] () { request.http_ctx.write(nullptr, 0); };

    MYSQL* mysql = nullptr;
    if (!async_pool_.empty()) {
        mysql = async_pool_.back();
        async_pool_.pop_back();
    }

    if (!mysql)
    {
      mysql = new MYSQL;
      mysql_init(mysql);
      std::cout << "mysql_init" << std::endl;
      mysql_options(mysql, MYSQL_OPT_NONBLOCK, 0);
      MYSQL* connection = nullptr;
      int status = mysql_real_connect_start(&connection, mysql, "127.0.0.1", "root", "sl_test_password", "silicon_test", 14550, NULL, 0);
      std::cout << "initialized" << std::endl;
      std::cout << "socket: " << mysql_get_socket(mysql) << std::endl;
      request.http_ctx.listen_to_new_fd(mysql_get_socket(mysql));
      while (status) {
        yield();
        status = mysql_real_connect_cont(&connection, mysql, status);
      }
      if (!connection)
        std::cout << "Failed to mysql_real_connect()" << std::endl;
    }
    else
    {
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
      std::cout <<  "mysql_real_query() returns error" << std::endl;

    /* This method cannot block. */
    MYSQL_RES *res;
    MYSQL_ROW row;
    res= mysql_use_result(mysql);
    if (!res)
      std::cout << "mysql_use_result() returns error" << std::endl;

    for (;;) {
      status= mysql_fetch_row_start(&row, res);
      while (status) {
        //status= wait_for_mysql(mysql, status);
        yield();
        status= mysql_fetch_row_cont(&row, res, status);
      }
      if (!row)
        break;
      //printf("%s: %s\n", row[0], row[1]);
      response.write(row[0]);
      break;
    }
    if (mysql_errno(mysql))
      std::cout << "Got error while retrieving rows" << std::endl;

    mysql_free_result(res);
    async_pool_.push_back(mysql);
    //mysql_close(mysql);
    //std::cout << "done" << std::endl;
  };



  http_serve(my_api, atoi(argv[1]));

}