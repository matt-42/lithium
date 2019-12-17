#pragma once

#include <atomic>
#include <iostream>
#include <cassert>
#include <cstring>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <thread>
#include <unordered_map>

#include <mysql.h>

#include <li/callable_traits/callable_traits.hh>
#include <li/metamap/metamap.hh>
#include <li/sql/sql_common.hh>
#include <li/sql/symbols.hh>
#include <li/sql/mysql_async_wrapper.hh>
#include <li/sql/mysql_statement.hh>


namespace li {

struct mysql_tag {};

struct mysql_database;

struct mysql_connection_data {

  ~mysql_connection_data() {
    // mysql_close(connection);
  }

  MYSQL* connection;
  std::unordered_map<std::string, std::shared_ptr<mysql_statement_data>> statements;
};

thread_local std::deque<std::shared_ptr<mysql_connection_data>> mysql_connection_pool;
thread_local std::deque<std::shared_ptr<mysql_connection_data>> mysql_connection_async_pool;
std::atomic<int> total_number_of_mysql_connections = 0;

template <typename B> // must be mysql_functions_blocking or mysql_functions_non_blocking
struct mysql_connection {

  typedef mysql_tag db_tag;

  inline mysql_connection(B mysql_wrapper, std::shared_ptr<li::mysql_connection_data> data)
      : mysql_wrapper_(mysql_wrapper), data_(data), stm_cache_(data->statements),
        con_(data->connection), broken_connection_(new bool(false)) {

    sptr_ = std::shared_ptr<int>((int*)42, [=](int* p) { 
      // if (*broken_connection_) 
      // {
      //   total_number_of_mysql_connections--;
      //   //std::cout << "discarding broken mysql connection." << std::endl;
      //   return;
      // }
      //std::cout << mysql_connection_async_pool.size() << std::endl;

      // need to cleanup the connection.
      if constexpr (B::is_blocking)
        mysql_connection_pool.push_back(data);
      else  
        mysql_connection_async_pool.push_back(data);

    });
  }


  long long int last_insert_rowid() { return mysql_insert_id(con_); }

  mysql_statement<B> operator()(const std::string& rq) { return prepare(rq)(); }

  mysql_statement<B> prepare(const std::string& rq) {
    auto it = stm_cache_.find(rq);
    if (it != stm_cache_.end())
    {
      return mysql_statement<B>{mysql_wrapper_, *it->second};
    }
    //std::cout << "prepare " << rq << std::endl;
    MYSQL_STMT* stmt = mysql_stmt_init(con_);
    if (!stmt)
    {
      *broken_connection_ = true;
      throw std::runtime_error(std::string("mysql_stmt_init error: ") + mysql_error(con_));
    }
    if (mysql_wrapper_.mysql_stmt_prepare(stmt, rq.data(), rq.size()))
    {
      std::cout << "error!!!" << std::endl;
      *broken_connection_ = true;
      throw std::runtime_error(std::string("mysql_stmt_prepare error: ") + mysql_error(con_));
    }

    auto pair = stm_cache_.emplace(rq, std::make_shared<mysql_statement_data>(stmt));
    return mysql_statement<B>{mysql_wrapper_, *pair.first->second};
  }

  template <typename T>
  inline std::string type_to_string(const T&, std::enable_if_t<std::is_integral<T>::value>* = 0) {
    return "INT";
  }
  template <typename T>
  inline std::string type_to_string(const T&,
                                    std::enable_if_t<std::is_floating_point<T>::value>* = 0) {
    return "DOUBLE";
  }
  inline std::string type_to_string(const std::string&) { return "MEDIUMTEXT"; }
  inline std::string type_to_string(const sql_blob&) { return "BLOB"; }
  template <unsigned S> inline std::string type_to_string(const sql_varchar<S>) {
    std::ostringstream ss;
    ss << "VARCHAR(" << S << ')';
    return ss.str();
  }

  B mysql_wrapper_;
  std::shared_ptr<mysql_connection_data> data_;
  std::unordered_map<std::string, std::shared_ptr<mysql_statement_data>>& stm_cache_;
  MYSQL* con_;
  std::shared_ptr<int> sptr_;
  std::shared_ptr<bool> broken_connection_;
};


struct mysql_database : std::enable_shared_from_this<mysql_database> {

  template <typename... O> inline mysql_database(O... opts) {

    auto options = mmm(opts...);
    static_assert(has_key(options, s::host), "open_mysql_connection requires the s::host argument");
    static_assert(has_key(options, s::database),
                  "open_mysql_connection requires the s::databaser argument");
    static_assert(has_key(options, s::user), "open_mysql_connection requires the s::user argument");
    static_assert(has_key(options, s::password),
                  "open_mysql_connection requires the s::password argument");

    host_ = options.host;
    database_ = options.database;
    user_ = options.user;
    passwd_ = options.password;
    port_ = get_or(options, s::port, 0);

    character_set_ = get_or(options, s::charset, "utf8");

    if (mysql_library_init(0, NULL, NULL))
      throw std::runtime_error("Could not initialize MySQL library.");
    if (!mysql_thread_safe())
      throw std::runtime_error("Mysql is not compiled as thread safe.");
  }

  ~mysql_database() { mysql_library_end(); }

  template <typename Y>
  inline mysql_connection<mysql_functions_non_blocking<Y>> connect(Y yield) {

    int ntry = 0;
    std::shared_ptr<mysql_connection_data> data = nullptr;
    while (!data)
    {
      // if (ntry > 20)
      //   throw std::runtime_error("Cannot connect to the database");
      ntry++;

      if (!mysql_connection_async_pool.empty()) {
        data = mysql_connection_async_pool.back();
        for (auto pair : data->statements)
        {
          //std::cout << "reset " << pair.first << std::endl; 
          //mysql_functions_non_blocking<decltype(yield)>{yield}.mysql_stmt_reset(pair.second->stmt_);
          mysql_functions_non_blocking<decltype(yield)>{yield}.mysql_stmt_free_result(pair.second->stmt_);
        }
        mysql_connection_async_pool.pop_back();
        yield.listen_to_fd(mysql_get_socket(data->connection));
      }
      else
      {
        // std::cout << total_number_of_mysql_connections << " connections. "<< std::endl;
        // if (total_number_of_mysql_connections > 40)
        // {
        //   //std::cout << "Waiting for a free mysql connection..." << std::endl;
        //   yield();
        //   continue;
        // }
        //std::cout << "NEW MYSQL CONNECTION "  << std::endl; 
        MYSQL* mysql;
        int mysql_fd = -1;
        int status;
        MYSQL* connection;
        //while (mysql_fd == -1)
        {
          mysql = new MYSQL;
          mysql_init(mysql);
          mysql_options(mysql, MYSQL_OPT_NONBLOCK, 0);
          connection = nullptr;
          status = mysql_real_connect_start(&connection, mysql, host_.c_str(), user_.c_str(), passwd_.c_str(),
                                            database_.c_str(), port_, NULL, 0);

          //std::cout << "after: " << mysql_get_socket(mysql) << " " << status == MYSQL_ << std::endl;
          mysql_fd = mysql_get_socket(mysql);
          if (mysql_fd == -1)
          {
            //std::cout << "Invalid mysql connection bad mysql_get_socket " << status << " " << mysql << std::endl;
            mysql_close(mysql);
            //usleep(1e6);
            yield();
            continue;
          }
        }
        if (status)
          yield.listen_to_fd(mysql_fd);
        while (status) {
          yield();
          status = mysql_real_connect_cont(&connection, mysql, status);
        }
        if (!connection)
        {
          //std::cout << "Error in mysql_real_connect_cont" << std::endl;
          yield();
          continue;
        }
          //throw std::runtime_error("Cannot connect to the database");
        mysql_set_character_set(mysql, character_set_.c_str());
        total_number_of_mysql_connections++;
        data = std::shared_ptr<mysql_connection_data>(new mysql_connection_data{mysql});
      }
    }
    assert(data);
    return mysql_connection(mysql_functions_non_blocking<decltype(yield)>{yield}, data);
  }

  inline mysql_connection<mysql_functions_blocking> connect() {
    std::shared_ptr<mysql_connection_data> data = nullptr;
    if (!mysql_connection_pool.empty()) {
      data = mysql_connection_pool.back();
      mysql_connection_pool.pop_back();
    }

    if (!data) {
      MYSQL* con_ = mysql_init(nullptr);
      con_ = mysql_real_connect(con_, host_.c_str(), user_.c_str(), passwd_.c_str(),
                                database_.c_str(), port_, NULL, 0);
      if (!con_)
        throw std::runtime_error("Cannot connect to the database");

      total_number_of_mysql_connections++;
      mysql_set_character_set(con_, character_set_.c_str());
      data = std::shared_ptr<mysql_connection_data>(new mysql_connection_data{con_});
    }

    assert(data);
    return mysql_connection(mysql_functions_blocking{}, data);
  }

  std::mutex mutex_;
  std::string host_, user_, passwd_, database_;
  unsigned int port_;
  std::deque<MYSQL*> _;
  std::string character_set_;
};


} // namespace li
