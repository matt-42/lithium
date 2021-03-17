#pragma once


#include <unordered_map>
#include <memory>

#include <li/sql/type_hashmap.hh>
#include <li/sql/internal/utils.hh>
#include <deque>

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

  // template <typename Y>
  void flush_current_query_result() {
    // println("flush_current_query_result ", current_result_id_);
    // ignore_current_query_result_ = true;
    // connection_->end_of_current_result(this->fiber_, false);
    // if (last_batch_end_id_ < current_result_id_)
    //   this->send_end_batch();
    while (true)
    {
      // if (PQconsumeInput(pgconn_) == 0)
      // {
      //   error_ = 1;
      //   break;
      // }
      // if (!PQisBusy(pgconn_))
      {
        // println("PQgetResult try flush");
        PGresult* res = PQgetResult(pgconn_);
        // println("PQgetResult flushed", res);
        // std::cout << 
        if (res) PQclear(res);

        // Break if:
        //   res is null means end of the query result.
        //   status is PGRES_PIPELINE_SYNC becaused it is not followed by a null PGresult.
        if (!res || PQresultStatus(res) == PGRES_PIPELINE_SYNC) 
          break;
      }
      // usleep(10);
    }
    // println("flush_current_query_result ", current_result_id_, " done  ");
    result_processing_in_progress_ = false;
  }

  void ignore_next_query_of_fiber(int fiber_id) {
    for (auto& q : batched_queries)
      if (q.fiber_id == fiber_id)
        q.ignore_result = true;
  }
  void ignore_result(int result_id) {
    for (int i = batched_queries.size() - 1; i >= 0; i--)
    // for (int i = 0; i < batched_queries.size(); i++)
      if (batched_queries[i].result_id == result_id)
      {
        batched_queries[i].ignore_result = true;
        return;
      }

    // println("ignore_result error: result does not exists: ", result_id);
    // assert(0);

  }

  template <typename Y>
  PGresult* wait_for_next_pgresult(Y& fiber, bool nothrow = false, bool async = true) {
    // println("WAIT for next PGresult    ====================== current result id is ", this->current_result_id_);
    while (true) {
      // println("PQconsumeInput");
      if (PQconsumeInput(pgconn_) == 0)
      {
        error_ = 1;          
        if (!nothrow)
          throw std::runtime_error(std::string("PQconsumeInput() failed: ") +
                                  PQerrorMessage(pgconn_));
        else
          std::cerr << "PQconsumeInput() failed: " << PQerrorMessage(pgconn_) << std::endl;
        assert(0);
      }

      if (PQisBusy(pgconn_)) {
        // std::cout << "isbusy" << std::endl;
        try {
          // In async mode, make sure the pq socket wakes up this fiber.
          if (async) {
            // println("         is busy => yield.  ======================");
            fiber.reassign_fd_to_fiber(PQsocket(pgconn_), fiber.continuation_idx);
            fiber.yield();
          }
        } catch (typename Y::exception_type& e) {
          // println("   wait for next PGresult ERROR WITH fiber", fiber.continuation_idx);

          // Free results.
          // Yield thrown a exception (probably because a closed connection).
          // Ignore batched result of this fiber.
          //this->ignore_next_query_of_fiber(fiber.continuation_idx);

          // Flush the remaining result of this query.
          this->flush_current_query_result();
          current_result_id_ = -1;
          // // Go to the next result.
          // end_of_current_result(fiber);

          throw std::move(e);
        }
      } else {
        // std::cout << "notbusy" << std::endl;
        // println("PQgetResult");
        PGresult* res = PQgetResult(pgconn_);
        auto status = PQresultStatus(res);
        if (status == PGRES_FATAL_ERROR and PQerrorMessage(pgconn_)[0] != 0)
        {
          PQclear(res);
          error_ = 1;          
          if (!nothrow)
            throw std::runtime_error(std::string("Postresql fatal error:") +
                                    PQerrorMessage(pgconn_));
          else
            std::cerr << "Postgresql FATAL error: " << PQerrorMessage(pgconn_) << std::endl;

          assert(0);
        }
        else if (status == PGRES_NONFATAL_ERROR)
        {
          PQclear(res);
          std::cerr << "Postgresql non fatal error: " << PQerrorMessage(pgconn_) << std::endl;
        }        
        else if (status == PGRES_PIPELINE_ABORTED) {
          PQclear(res);
          std::cerr << "Postgresql batch aborted: " << PQerrorMessage(pgconn_) << std::endl;        
        }

        // if (res == nullptr)
        //   println("   PQgetResult returned NULL");

        // if (status == PGRES_PIPELINE_SYNC)
        //   println("   PQgetResult returned PGRES_PIPELINE_SYNC");
        // println("    wait for next result OK.");
        return res;
      }
    }
  }
  struct batch_query_info {int fiber_id; int result_id; bool ignore_result; bool is_batch_end=false; };

  void send_end_batch() {
    // if (batched_queries.size() && batched_queries.back().result_id == last_batch_end_id_) return;

    // println("send_end_batch: batch size: ", batched_queries.back().result_id - last_batch_end_id_);
    // println("PQsendEndBatch"); 
    if (0 == PQpipelineSync(this->pgconn_))
      std::cerr << "PQpipelineSync error"  << std::endl; 
    int result_id = this->next_result_id++;// batched_queries.size() == 0 ? 1 : batched_queries.back().result_id + 1;
    last_batch_end_id_ = result_id;
    batched_queries.push_back(batch_query_info{0, result_id, true, true});
  }
  // Go to next batched query result
  template <typename Y>
  int batch_query(Y& fiber, bool ignore_result = false) {

    int result_id = next_result_id++;//batched_queries.size() == 0 ? 1 : batched_queries.back().result_id + 1;
    // println("batch query ", result_id, " ignore: ", ignore_result, "queue size", batched_queries.size());
    batched_queries.push_back(batch_query_info{fiber.continuation_idx, result_id, ignore_result});

    // if (batched_queries.back().result_id - last_batch_end_id_ > 15000)
    //   this->send_end_batch();
    // if (!end_batch_defered)
    // {
    //   end_batch_defered = true;
    //   // std::cout << "defer endbatch" << std::endl;
    //   fiber.defer([this] { 
    //     end_batch_defered = false;
    //     this->send_end_batch();
    //     });
    // }
    // while (batched_queries.size() > 15000)
    //   fiber.yield();
    return result_id;
  }

  // Get the next query for reading the results.
  // call send_end_batch if the query is in the batch currently
  // being accumulated.
  inline auto pq_get_next_query() {

    auto query = batched_queries.front();
    batched_queries.pop_front();
    current_result_id_ = query.result_id;

    // current_result_id_ = result_id;
    if (current_result_id_ > last_batch_end_id_) this->send_end_batch();
    
    // println("pq_get_next_query");
    // if (0 == PQbatchProcessQueue(pgconn_))
    // {
    //   std::cerr << "PQgetNextQuery error : " <<  PQerrorMessage(pgconn_) << std::endl;
    //   assert(0);
    //   throw std::runtime_error(std::string("PQgetNextQuery error : ") + PQerrorMessage(pgconn_));
    // }
    return query;
  }

  template <typename Y>
  void flush_ignored_results(Y& fiber, bool async = true) {

    // println("flush_ignored_results");

    assert(!result_processing_in_progress_);
    
    result_processing_in_progress_ = true;
    // println(" flush_ignored_results: result_processing_in_progress_ = true");

    // if (batched_queries.size() > 0 && batched_queries.front().ignore_result)
    //   this->send_end_batch();

    while (batched_queries.size() > 0 && batched_queries.front().ignore_result) {


      // std::cout << "PQgetNextQuery" << std::endl; 
      auto query = this->pq_get_next_query();
      // println(" ignore ", query.result_id);
      assert(query.ignore_result);

      // println("ignore result ", query.result_id);
      while (true)
      {
        // println("wait one result");
        PGresult* res = this->wait_for_next_pgresult(fiber, false, async);
        // println("one result ok");
        if (res)
        {
          // if (query.is_batch_end) println("got PGRES_BATCH_END");
          int status = PQresultStatus(res);
          if (query.is_batch_end && status != PGRES_PIPELINE_SYNC)
          {
            std::cerr << " got batch end but PQresultStatus(res) == " << status << ". It should be PGRES_PIPELINE_SYNC: " << PGRES_PIPELINE_SYNC << std::endl;
          }
          assert(!query.is_batch_end || status == PGRES_PIPELINE_SYNC);
          PQclear(res);
          if (query.is_batch_end)
            break; // PGRES_PIPELINE_SYNC is not followed by a null PGresult.
        }
        else break;
      }
      // println(" ignore ", query.result_id, " OK.");

    }
    // println("end flush_ignored_results");
    if (batched_queries.size() == 0)
    {
      // std::cout << "current_result_id_ = -1" << std::endl; 
      current_result_id_ = -1;        
    }

    // println("after flush ignore result, queue size == ", batched_queries.size());

    result_processing_in_progress_ = false;
  }

  // The current result is totally consumed. Go to the next one.
  // If another fiber is waiting for the next one, defer the wake up of this fiber. 
  template <typename Y>
  void end_of_current_result(Y& fiber, bool async = true) {
    // println("end_of_current_result: ", this->current_result_id_);
    // assert(result_processing_in_progress_);
    result_processing_in_progress_ = false;

    // Ignored result.
    flush_ignored_results(fiber, async);

    if (batched_queries.size() > 0)
    {
      auto query = this->pq_get_next_query();
      assert(!query.ignore_result);
      // std::cout << "current_result_id_ = " << current_result_id_ << std::endl; 

      // Wake up the fiber waiting for this query if not the calling one.
      if (query.fiber_id != fiber.continuation_idx)
      {
        // std::cout << " Next fiber to read result is " << query.fiber_id << " with result id " << this->current_result_id_ << std::endl;
        // fiber.reassign_fd_to_fiber(PQsocket(pgconn_), query.fiber_id);
        fiber.defer_fiber_resume(query.fiber_id);
      }
    }
    else
    {
      // println("   end_of_current_result: queue is now empty");
      // std::cout << "current_result_id_ = -1" << std::endl; 
      current_result_id_ = -1;        
    }

    // Ready for processing next result.
    // println(" end of result OK. current_result_id_: ", current_result_id_);

    result_processing_in_progress_ = false;

  }

  template <typename Y>
  void wait_for_result(Y& fiber, int result_id) {
    // println("wait for result", result_id, " current is: ", current_result_id_);
    if (current_result_id_ == result_id) return;

        
    if (current_result_id_ == -1)
    {
      // this->flush_ignored_results(fiber);
      // this->pg_get_next_query();
      this->end_of_current_result(fiber);
    }
    while (current_result_id_ != result_id)
    {
      // try {
      fiber.yield();      
      // } catch 
    }

    // Send endbatch if the requested result is in the current (not ended) batch.
    // if (result_id > last_batch_end_id_) this->send_end_batch();

    // println("wait_for_result ", result_id, " ok: current_result_id_: ", current_result_id_);
    // assert(!result_processing_in_progress_);
    // Flush any ignored result.
    // flush_ignored_results(fiber);

    // println(" PQ GET NEXT QUERY ", current_result_id_);
    // this->pq_get_next_query();
    // println("wait for result done.");

    assert(!result_processing_in_progress_);
    // println(" wait for result: result_processing_in_progress_ = true");
    result_processing_in_progress_ = true;
  }

  PGconn* pgconn_ = nullptr;
  int fd = -1;
  std::unordered_map<std::string, std::shared_ptr<pgsql_statement_data>> statements;
  type_hashmap<std::shared_ptr<pgsql_statement_data>> statements_hashmap;
  std::deque<batch_query_info> batched_queries;
  int current_result_id_ = -1;
  int error_ = 0;
  int next_result_id = 1;
  bool end_batch_defered = false;
  bool result_processing_in_progress_ = false;
  int last_batch_end_id_ = -1;
};

}
