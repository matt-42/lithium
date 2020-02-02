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
#include <boost/lexical_cast.hpp>
#include <sys/epoll.h>
#include <mysql.h>

#include <li/callable_traits/callable_traits.hh>
#include <li/metamap/metamap.hh>
#include <li/sql/sql_common.hh>
#include <li/sql/symbols.hh>
#include <li/sql/mysql_async_wrapper.hh>
#include <li/sql/mysql_statement.hh>
#include <li/sql/type_hashmap.hh>

namespace li {

int max_mysql_connections_per_thread = 200;

struct mysql_tag {};

struct mysql_database;

struct mysql_connection_data {

  ~mysql_connection_data() {
    mysql_close(connection);
  }

  MYSQL* connection;
  std::unordered_map<std::string, std::shared_ptr<mysql_statement_data>> statements;
  type_hashmap<std::shared_ptr<mysql_statement_data>> statements_hashmap;
};

thread_local std::deque<std::shared_ptr<mysql_connection_data>> mysql_connection_pool;
thread_local std::deque<std::shared_ptr<mysql_connection_data>> mysql_connection_async_pool;
thread_local int total_number_of_mysql_connections = 0;

template <typename B> // must be mysql_functions_blocking or mysql_functions_non_blocking
struct mysql_result {

  B& mysql_wrapper_;
  MYSQL* con_;
  std::shared_ptr<int> connection_status_;
  MYSQL_RES* result_ = nullptr;

  template <typename T>
  T read() {
    result_ = mysql_use_result(con_);
    MYSQL_ROW row = mysql_wrapper_.mysql_fetch_row(connection_status_, result_);
    return boost::lexical_cast<T>(row[0]);
    mysql_wrapper_.mysql_free_result(connection_status_, result_); 
  } 

  void wait() {}

};

template <typename B> // must be mysql_functions_blocking or mysql_functions_non_blocking
struct mysql_connection {

  typedef mysql_tag db_tag;

  inline mysql_connection(B mysql_wrapper, std::shared_ptr<li::mysql_connection_data> data)
      : mysql_wrapper_(mysql_wrapper), data_(data), stm_cache_(data->statements),
        con_(data->connection){

    connection_status_ = std::shared_ptr<int>(new int(0), [=](int* p) mutable { 

      if (*p or (mysql_connection_pool.size() + mysql_connection_async_pool.size())  > max_mysql_connections_per_thread)
      {
      //  if constexpr (!B::is_blocking)
      //    mysql_wrapper.yield_.unsubscribe(mysql_get_socket(data->connection));
       total_number_of_mysql_connections--;
        //std::cerr << "Discarding broken mysql connection." << std::endl;
        return;
      }
      
      if constexpr (B::is_blocking)
        mysql_connection_pool.push_back(data);
      else  
        mysql_connection_async_pool.push_back(data);

    });
  }


  long long int last_insert_rowid() { return mysql_insert_id(con_); }

  mysql_result<B> operator()(const std::string& rq) { 
    mysql_wrapper_.mysql_real_query(connection_status_, con_, rq.c_str(), rq.size());
    return mysql_result<B>{mysql_wrapper_, con_, connection_status_};
  }

  template <typename F, typename... K>
  mysql_statement<B> cached_statement(F f, K... keys) {
    if (data_->statements_hashmap(f).get() == nullptr)
    {
      mysql_statement<B> res = prepare(f());
      data_->statements_hashmap(f, keys...) = res.data_.shared_from_this();
      return res;
    }
    else
      return mysql_statement<B>{mysql_wrapper_, 
                                *data_->statements_hashmap(f, keys...), 
                                 connection_status_};
  }

  mysql_statement<B> prepare(const std::string& rq) {
    auto it = stm_cache_.find(rq);
    if (it != stm_cache_.end())
    {
      //mysql_wrapper_.mysql_stmt_free_result(it->second->stmt_);
      //mysql_wrapper_.mysql_stmt_reset(it->second->stmt_);
      return mysql_statement<B>{mysql_wrapper_, *it->second, connection_status_};
    }
    //std::cout << "prepare " << rq << std::endl;
    MYSQL_STMT* stmt = mysql_stmt_init(con_);
    if (!stmt)
    {
      *connection_status_ = true;
      throw std::runtime_error(std::string("mysql_stmt_init error: ") + mysql_error(con_));
    }
    if (mysql_wrapper_.mysql_stmt_prepare(connection_status_, stmt, rq.data(), rq.size()))
    {
      *connection_status_ = true;
      throw std::runtime_error(std::string("mysql_stmt_prepare error: ") + mysql_error(con_));
    }

    auto pair = stm_cache_.emplace(rq, std::make_shared<mysql_statement_data>(stmt));
    return mysql_statement<B>{mysql_wrapper_, *pair.first->second, connection_status_};
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
  std::shared_ptr<int> connection_status_;
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
  inline mysql_connection<mysql_functions_non_blocking<Y>> connect(Y& fiber) {

    //std::cout << "nconnection " << total_number_of_mysql_connections << std::endl;
    int ntry = 0;
    std::shared_ptr<mysql_connection_data> data = nullptr;
    while (!data)
    {
      // if (ntry > 20)
      //   throw std::runtime_error("Cannot connect to the database");
      ntry++;

      if (!mysql_connection_async_pool.empty()) {
        data = mysql_connection_async_pool.back();
        mysql_connection_async_pool.pop_back();
        fiber.epoll_add(mysql_get_socket(data->connection), EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET);
      }
      else
      {
        // std::cout << total_number_of_mysql_connections << " connections. "<< std::endl;
        if (total_number_of_mysql_connections > max_mysql_connections_per_thread)
        {
          //std::cout << "Waiting for a free mysql connection..." << std::endl;
          fiber.yield();
          continue;
        }
        total_number_of_mysql_connections++;
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
            total_number_of_mysql_connections--;
            //usleep(1e6);
            fiber.yield();
            continue;
          }
        }
        if (status)
          fiber.epoll_add(mysql_fd, EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET);
        while (status) try {
          fiber.yield();
          status = mysql_real_connect_cont(&connection, mysql, status);
        }  catch (typename Y::exception_type& e) {
          // Yield thrown a exception (probably because a closed connection).
          // std::cerr << "Warning: yield threw an exception while connecting to mysql: "
          //  << total_number_of_mysql_connections << std::endl;
          total_number_of_mysql_connections--;
          mysql_close(mysql);
          throw std::move(e);
        }
        if (!connection)
        {
          //std::cout << "Error in mysql_real_connect_cont" << std::endl;
          total_number_of_mysql_connections--;
          fiber.yield();
          continue;
        }
          //throw std::runtime_error("Cannot connect to the database");
        mysql_set_character_set(mysql, character_set_.c_str());
        data = std::shared_ptr<mysql_connection_data>(new mysql_connection_data{mysql});
      }
    }
    assert(data);
    return std::move(mysql_connection(mysql_functions_non_blocking<Y>{fiber}, data));
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

      //total_number_of_mysql_connections++;
      mysql_set_character_set(con_, character_set_.c_str());
      data = std::shared_ptr<mysql_connection_data>(new mysql_connection_data{con_});
    }

    assert(data);
    return std::move(mysql_connection(mysql_functions_blocking{}, data));
  }

  std::mutex mutex_;
  std::string host_, user_, passwd_, database_;
  unsigned int port_;
  std::deque<MYSQL*> _;
  std::string character_set_;
};


} // namespace li
