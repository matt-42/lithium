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

  struct batch_query_info {int fiber_id; int result_id; bool ignore_result; bool is_batch_end=false; };

  // Go to next batched query result
  template <typename Y>
  int batch_query(Y& fiber, bool ignore_result = false) {
    // std::cout << "batch query " << ignore_result << std::endl; 
    int result_id = next_result_id++;//batched_queries.size() == 0 ? 1 : batched_queries.back().result_id + 1;
    batched_queries.push_back(batch_query_info{fiber.continuation_idx, result_id, ignore_result});
    // std::cout << "PQqueriesInBatch(pgconn_) " << PQqueriesInBatch(pgconn_) << " batched_queries.size() " << batched_queries.size() << std::endl; 
    // assert(PQqueriesInBatch(pgconn_) == batched_queries.size());
    // if (batched_queries.size() == 1)
    // {
    //     std::cout << "PQsendEndBatch" << std::endl; 
    //     if (0 == PQsendEndBatch(this->pgconn_))
    //       std::cerr << "PQsendEndBatch error"  << std::endl; 
    // }
    if (!end_batch_defered)
    {
      end_batch_defered = true;
      // std::cout << "defer endbatch" << std::endl;
      fiber.defer([this] { 
        // std::cout << "PQsendEndBatch" << std::endl; 
        if (0 == PQsendEndBatch(this->pgconn_))
          std::cerr << "PQsendEndBatch error"  << std::endl; 
        int result_id = this->next_result_id++;// batched_queries.size() == 0 ? 1 : batched_queries.back().result_id + 1;
        batched_queries.push_back(batch_query_info{0, result_id, true, true});
        end_batch_defered = false;
        // std::cout << "PQqueriesInBatch(pgconn_) " << PQqueriesInBatch(pgconn_) << " batched_queries.size() " << batched_queries.size() << std::endl; 
        // assert(PQqueriesInBatch(pgconn_) == batched_queries.size());

        // this->end_of_current_result(fiber);        
        // if (0 == PQgetNextQuery(pgconn_))
        //   std::cerr << "PQgetNextQuery error" << std::endl;
         
        });
    }
    return result_id;
  }

  // The current result is totally consumed. Go to the next one.
  template <typename Y>
  void end_of_current_result(Y& fiber) {
    // std::cout << " end_of_current_result fiber " << fiber.continuation_idx << std::endl;
    if (batched_queries.size() == 0) 
    {
      // std::cout << "queue empty" << std::endl; 
      current_result_id = -1;        
      return;
    }

    // Get next query.
    bool ignore_result = true;
    while (ignore_result == true && batched_queries.size() > 0) {
      // std::cout << "PQgetNextQuery" << std::endl; 
      if (0 == PQgetNextQuery(pgconn_))
      {
        std::cerr << "PQgetNextQuery error : " <<  PQerrorMessage(pgconn_) << std::endl;
        throw std::runtime_error(std::string("PQgetNextQuery error : ") + PQerrorMessage(pgconn_));
      }
      ignore_result = batched_queries.front().ignore_result;
      this->current_result_id = batched_queries.front().result_id;

      if (batched_queries.front().is_batch_end)
      {
        PGresult* res = pg_wait_for_next_result(pgconn_, fiber, error_, false);
        assert(res);
        assert(PQresultStatus(res) == PGRES_BATCH_END);
        PQclear(res);
      }
      else if (ignore_result)
        {
          // std::cout << "ignore result " << std::endl;
          while (true)
          {
            PGresult* res = pg_wait_for_next_result(pgconn_, fiber, error_, false);
            if (res)
              PQclear(res);
            else break;
          }
        }
      else
      {
        // Wake up the fiber waiting for this query if not the calling one.
        if (batched_queries.front().fiber_id != fiber.continuation_idx)
        {
          // std::cout << " Next fiber to read result is " << batched_queries.front().fiber_id << " with result id " << this->current_result_id << std::endl;
          fiber.reassign_fd_to_fiber(PQsocket(pgconn_), batched_queries.front().fiber_id);
          fiber.defer_fiber_resume(batched_queries.front().fiber_id);
        }
      }
      batched_queries.pop_front();
      // assert(PQqueriesInBatch(pgconn_) == batched_queries.size());
      // std::cout << "PQqueriesInBatch(pgconn_) " << PQqueriesInBatch(pgconn_) << " batched_queries.size() " << batched_queries.size() << std::endl; 
    };

    if (batched_queries.size() == 0 && ignore_result) 
    {
      // std::cout << "current_result_id = -1" << std::endl; 
      current_result_id = -1;        
      return;
    }

  }

  PGconn* pgconn_ = nullptr;
  int fd = -1;
  std::unordered_map<std::string, std::shared_ptr<pgsql_statement_data>> statements;
  type_hashmap<std::shared_ptr<pgsql_statement_data>> statements_hashmap;
  std::deque<batch_query_info> batched_queries;
  int current_result_id = -1;
  int error_ = 0;
  int next_result_id = 1;
  bool end_batch_defered = false;
};

}
