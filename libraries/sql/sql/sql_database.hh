#pragma once

#include <deque>
#include <unordered_map>

namespace li {
// thread local map of sql_database<I> -> sql_database_thread_local_data<I>;
// This is used to store the thread local async connection pool.
thread_local std::unordered_map<int, void*> sql_thread_local_data;
thread_local int database_id_counter = 0;

template <typename I> struct sql_database_thread_local_data {

  typedef typename I::connection_data_type connection_data_type;

  // Async connection pools.
  std::deque<std::shared_ptr<connection_data_type>> async_connections_;

  int n_connections_on_this_thread_ = 0;
};

struct active_yield {
  typedef std::runtime_error exception_type;
  void epoll_add(int, int) {}
  void epoll_mod(int, int) {}
  void yield() {}
};

template <typename I> struct sql_database {
  I impl;

  typedef typename I::connection_data_type connection_data_type;
  typedef typename I::db_tag db_tag;

  // Sync connections pool.
  std::deque<std::shared_ptr<connection_data_type>> sync_connections_;
  // Sync connections mutex.
  std::mutex sync_connections_mutex_;

  int n_sync_connections_ = 0;
  int max_sync_connections_ = 0;
  int max_async_connections_per_thread_ = 0;
  int database_id_ = 0;

  template <typename... O> sql_database(O&&... opts) : impl(std::forward<O>(opts)...) {
    auto options = mmm(opts...);
    max_async_connections_per_thread_ = get_or(options, s::max_async_connections_per_thread, 200);
    max_sync_connections_ = get_or(options, s::max_sync_connections, 2000);

    this->database_id_ = database_id_counter++;
  }

  ~sql_database() {
    auto it = sql_thread_local_data.find(this->database_id_);
    if (it != sql_thread_local_data.end())
    {
      delete (sql_database_thread_local_data<I>*) sql_thread_local_data[this->database_id_];
      sql_thread_local_data.erase(this->database_id_);
    }
  }

  auto& thread_local_data() {
    auto it = sql_thread_local_data.find(this->database_id_);
    if (it == sql_thread_local_data.end())
    {
      auto data = new sql_database_thread_local_data<I>;
      sql_thread_local_data[this->database_id_] = data;
      return *data;
    }
    else
      return *(sql_database_thread_local_data<I>*) it->second;
  }
  /**
   * @brief Build aa new database connection. The connection provides RAII: it will be
   * placed back in the available connection pool whenever its constructor is called.
   *
   * @param fiber the fiber object providing the 3 non blocking logic methods:
   *
   *    - void epoll_add(int fd, int flags); // Make the current epoll fiber wakeup on
   *                                            file descriptor fd
   *    - void epoll_mod(int fd, int flags); // Modify the epoll flags on file
   *                                            descriptor fd
   *    - void yield() // Yield the current epoll fiber.
   *
   * @return the new connection.
   */
  template <typename Y> inline auto connect(Y& fiber) {

    auto pool = [this] {

      if constexpr (std::is_same_v<Y, active_yield>) // Synchonous mode
        return make_metamap_reference(
            s::connections = this->sync_connections_,
            s::n_connections = this->n_sync_connections_,
            s::max_connections = this->max_sync_connections_);
      else  // Asynchonous mode
        return make_metamap_reference(
            s::connections = this->thread_local_data().async_connections_,
            s::n_connections =  this->thread_local_data().n_connections_on_this_thread_,
            s::max_connections = this->max_async_connections_per_thread_);
    }();

    std::shared_ptr<connection_data_type> data = nullptr;
    while (!data) {

      if (!pool.connections.empty()) {
        auto lock = [&pool, this] {
          if constexpr (std::is_same_v<Y, active_yield>)
            return std::lock_guard<std::mutex>(this->sync_connections_mutex_);
          else return 0;
        }();
        data = pool.connections.back();
        pool.connections.pop_back();
        fiber.epoll_add(impl.get_socket(data), EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET);
      } else {
        if (pool.n_connections >= pool.max_connections) {
          if constexpr (std::is_same_v<Y, active_yield>)
            throw std::runtime_error("Maximum number of sql connection exeeded.");
          else
            // std::cout << "Waiting for a free sql connection..." << std::endl;
            fiber.yield();
          continue;
        }
        pool.n_connections++;

        try {

          data = impl.new_connection(fiber);
        } catch (typename Y::exception_type& e) {
          pool.n_connections--;
          throw std::move(e);
        }

        if (!data)
          pool.n_connections--;
      }
    }
    assert(data);
    return impl.scoped_connection(
        fiber, data, [pool, this](std::shared_ptr<connection_data_type> data, int error) {
          if (!error && pool.connections.size() < pool.max_connections) {
            auto lock = [&pool, this] {
              if constexpr (std::is_same_v<Y, active_yield>)
                return std::lock_guard<std::mutex>(this->sync_connections_mutex_);
              else return 0;
            }();

            pool.connections.push_back(data);
            
          } else {
            if (pool.connections.size() > pool.max_connections)
              std::cerr << "Error: connection pool size " << pool.connections.size()
                        << " exceed pool max_connections " << pool.max_connections << std::endl;
            pool.n_connections--;
          }
        });
  }

  /**
   * @brief Provide a new mysql blocking connection. The connection provides RAII: it will be
   * placed back in the available connection pool whenver its constructor is called.
   *
   * @return the connection.
   */
  inline auto connect() { active_yield yield; return this->connect(yield); }
};

}