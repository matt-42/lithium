#include <deque>
#include <unordered_map>

// thread local map of sql_database<I> -> sql_database_thread_local_data<I>;
// This is used to store the thread local async connection pool.
thread_local std::unordered_map<int, void*> sql_thread_local_data;
thread_local int database_id_counter = 0;

template <typename I> struct sql_database_thread_local_data {

  using typename I::connection_data_type connection_data_type;

  // Async connection pools.
  std::deque<std::shared_ptr<mysql_connection_data>> async_connections_;

  int n_connections_on_this_thread_ = 0;
}

struct active_yield {
  typedef std::runtime_error exception_type;
  void epoll_add(int, int) {}
  void epoll_mod(int, int) {}
  void yield() {}
};

template <typename I> struct sql_database {
  I impl;

  // Sync connections pool.
  std::deque<std::shared_ptr<mysql_connection_data>> sync_connections_;
  // Sync connections mutex.
  std::mutex sync_connections_mutex_;

  int n_sync_connections_ = 0;
  int max_sync_connections_ = 0;
  int max_sql_connections_per_thread_ = 0;
  int database_id_ = 0;

  using typename I::connection_data_type connection_data_type;

  template <typename... O> sql_database(O&&... opts) : impl(std::forward<O>(opts)...) {
    max_async_connections_per_thread_ = get_or(option, s::max_async_connections_per_thread, 200);
    max_sync_connections_ = get_or(option, s::max_sync_connections, 2000);

    this->database_id_ = database_id_counter++:
  }

  auto& thread_local_data() {
    return *(sql_database_thread_local_data<I>*)sql_thread_local_data[this->database_id_];
  }
  /**
   * @brief Provide a new mysql non blocking connection. The connection provides RAII: it will be
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
    auto pool = [] {
      if constexpr (std::is_same_v<Y, active_yield>) // Synchonous mode
        return make_metamap_reference(
            s::connections = this->sync_connections_,
            s::lock_pool =
                [] { return std::scoped_lock<std::mutex>(this->sync_connections_mutex_); },
            s::n_connections = this->n_sync_connections_,
            s::max_connections = this->max_sync_connections_);
      else  // Asynchonous mode
        return make_metamap_reference(
            s::connections = this->thread_local_data().async_connections_,
            s::lock_pool = 0,
            s::n_connections =  this->thread_local_data().n_connections_on_this_thread_,
            s::max_connections = this->max_async_connections_per_thread_);
    }();

    // auto& connection_pool = [] {
    //   if constexpr (std::is_same_v<Y, active_yield>)
    //     return this->sync_connections_;
    //   else
    //     return this->thread_local_data().async_connections_;
    // }();

    // auto lock_pool = [] {
    //   if constexpr (std::is_same_v<Y, active_yield>)
    //     return std::scoped_lock<std::mutex>(this->sync_connections_mutex_);
    //   else
    //     return 0;
    // };

    // int& n_connections = [] {
    //   if constexpr (std::is_same_v<Y, active_yield>)
    //     return this->n_sync_connections_;
    //   else
    //     return this->thread_local_data().n_connections_on_this_thread_;
    // };
    // int max_connections = [] {
    //   if constexpr (std::is_same_v<Y, active_yield>)
    //     return this->max_sync_connections_;
    //   else
    //     return impl.max_connections_per_thread_;
    // };

    // int ntry = 0;
    std::shared_ptr<connection_data_type> data = nullptr;
    while (!data) {
      // if (ntry > 20)
      //   throw std::runtime_error("Cannot connect to the database");
      ntry++;

      if (!pool.connections.empty()) {
        auto lock = pool.lock_pool();
        data = pool.connections.back();
        pool.connections.pop_back();
        fiber.epoll_add(impl.get_socket(data), EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET);
      } else {
        // std::cout << total_number_of_mysql_connections << " connections. "<< std::endl;
        if (pool.n_connections > pool.max_connections) {
          if constexpr (std::is_same_v<Y, active_yield>)
            throw std::runtime_error("Maximum number of sql connection exeeded.");
          else
            // std::cout << "Waiting for a free mysql connection..." << std::endl;
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
        fiber, data, [&](std::shared_ptr<connection_data_type> data, int error) {
          if (!error && pool.connections.size() < pool.max_connections) {
            auto lock = pool.lock_pool();
            pool.connections.push_back(data);
          } else {
            if (pool.connections.size() >= pool.max_connections)
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
  inline auto connect() { this->connect(active_yield); }
}