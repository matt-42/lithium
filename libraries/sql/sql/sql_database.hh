#pragma once

#include <deque>
#include <unordered_map>

namespace li {
  
// thread local map of sql_database<I>* -> sql_database_thread_local_data<I>*;
// This is used to store the thread local async connection pool.
// void* is used instead of concrete types to handle different I parameter.
#ifndef _WIN32
thread_local std::unordered_map<void*, void*> sql_thread_local_data [[gnu::weak]];
#else
__declspec(selectany) thread_local std::unordered_map<void*, void*> sql_thread_local_data; 
#endif

template <typename I> struct sql_database_thread_local_data {

  typedef typename I::connection_data_type connection_data_type;

  // Async connection pools.
  std::deque<connection_data_type*> async_connections_;

  int n_connections_on_this_thread_ = 0;
  std::deque<int> fibers_waiting_for_connections_;
};

struct active_yield {
  typedef std::runtime_error exception_type;
  int fiber_id = 0;
  inline void defer(std::function<void()>) {}
  inline void defer_fiber_resume(int fiber_id) {}
  inline void reassign_fd_to_this_fiber(int fd) {}

  inline void epoll_add(int fd, int flags) {}
  inline void epoll_mod(int fd, int flags) {}
  inline void yield() {}
};

template <typename I> struct sql_database {
  I impl;

  typedef typename I::connection_data_type connection_data_type;
  typedef typename I::db_tag db_tag;

  // Sync connections pool.
  std::deque<connection_data_type*> sync_connections_;
  // Sync connections mutex.
  std::mutex sync_connections_mutex_;

  int n_sync_connections_ = 0;
  int max_sync_connections_ = 0;
  int max_async_connections_per_thread_ = 0;

  template <typename... O> sql_database(O&&... opts) : impl(std::forward<O>(opts)...) {
    auto options = mmm(opts...);
    max_async_connections_per_thread_ = get_or(options, s::max_async_connections_per_thread, 200);
    max_sync_connections_ = get_or(options, s::max_sync_connections, 2000);

  }

  ~sql_database() {
    clear_connections();
  }

  void clear_connections() {
    auto it = sql_thread_local_data.find(this);
    if (it != sql_thread_local_data.end())
    {
      auto store = (sql_database_thread_local_data<I>*) it->second;
      for (auto ptr : store->async_connections_)
        delete ptr;
      delete store;
      sql_thread_local_data.erase(this);
    }

    std::lock_guard<std::mutex> lock(this->sync_connections_mutex_);
    for (auto* ptr : this->sync_connections_)
      delete ptr;
    sync_connections_.clear();
    n_sync_connections_ = 0;
  }

  auto& thread_local_data() {
    auto it = sql_thread_local_data.find(this);
    if (it == sql_thread_local_data.end())
    {
      auto data = new sql_database_thread_local_data<I>;
      sql_thread_local_data[this] = data;
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

    auto& tldata = this->thread_local_data();
    auto pool = [this, &tldata] {

      if constexpr (std::is_same_v<Y, active_yield>) // Synchonous mode
        return make_metamap_reference(
            s::connections = this->sync_connections_,
            s::n_connections = this->n_sync_connections_,
            s::max_connections = this->max_sync_connections_);
      else  // Asynchonous mode
        return make_metamap_reference(
            s::connections = tldata.async_connections_,
            s::n_connections =  tldata.n_connections_on_this_thread_,
            s::max_connections = this->max_async_connections_per_thread_,
            s::waiting_list = tldata.fibers_waiting_for_connections_);
    }();

    connection_data_type* data = nullptr;
    bool reuse = false;
    time_t start_time = time(NULL);
    while (!data) {
      if (!pool.connections.empty()) {
        auto lock = [&pool, this] {
          if constexpr (std::is_same_v<Y, active_yield>)
            return std::lock_guard<std::mutex>(this->sync_connections_mutex_);
          else return 0;
        }();
        data = pool.connections.back();
        pool.connections.pop_back();
        reuse = true;
      } else {
        if (pool.n_connections >= pool.max_connections) {
          if constexpr (std::is_same_v<Y, active_yield>)
            throw std::runtime_error("Maximum number of sql connection exeeded.");
          else
          {
            // std::cout << "Waiting for a free sql connection..." << std::endl;
            //  pool.waiting_list.push_back(fiber.fiber_id);
            fiber.yield();
          }
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

      if (time(NULL) > start_time + 10)
        throw std::runtime_error("Timeout: Cannot connect to the database."); 
    }

    assert(data);
    assert(data->error_ == 0);
    
    auto sptr = std::shared_ptr<connection_data_type>(data, [pool, this, &fiber](connection_data_type* data) {
          if (!data->error_ && pool.connections.size() < pool.max_connections) {
            auto lock = [&pool, this] {
              if constexpr (std::is_same_v<Y, active_yield>)
                return std::lock_guard<std::mutex>(this->sync_connections_mutex_);
              else return 0;
            }();

            pool.connections.push_back(data);
            if constexpr (!std::is_same_v<Y, active_yield>)
             if (pool.waiting_list.size())
             {
               int next_fiber_id = pool.waiting_list.front();
               pool.waiting_list.pop_front();
               fiber.defer_fiber_resume(next_fiber_id);
             }
           
          } else {
            // This is not an error since connection pool.max_connections can vary during execution.
            // It is ok just to discard extraneous in order to reach a lower pool.max_connections.
            // if (pool.connections.size() >= pool.max_connections)
            //   std::cerr << "Error: connection pool size " << pool.connections.size()
            //             << " exceed pool max_connections " << pool.max_connections << " " << pool.n_connections<< std::endl;
            pool.n_connections--;
            delete data;
          }
        });

    if (reuse) 
      fiber.reassign_fd_to_this_fiber(impl.get_socket(sptr));

    return impl.scoped_connection(fiber, sptr);
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