#include <lithium.hh>

#include "symbols.hh"
using namespace li;

template< typename T, typename Pred >
typename std::vector<T>::iterator
    insert_sorted( std::vector<T> & vec, T&& item, Pred&& pred )
{
    return vec.emplace
        ( 
           std::upper_bound( vec.begin(), vec.end(), std::forward<T>(item), pred ),
           std::forward<T>(item)
        );
}
template <typename B> void escape_html_entities_(B& buffer, const std::string& data) {

  int last_pos_writen = -1;

  auto flush_until = [&] (int pos) {
    // if (pos == last_pos_writen) return;
    buffer << std::string_view(data.c_str() + last_pos_writen + 1, pos - last_pos_writen);
    // last_pos_writen = pos;
  };

  for (size_t pos = 0; pos != data.size(); ++pos) {
    switch (data[pos]) {
    case '&':
      flush_until(pos-1);
      buffer << "&amp;";
      last_pos_writen = pos;
      break;
    case '\"':
      flush_until(pos-1);
      buffer << "&quot;";
      last_pos_writen = pos;
      break;
    case '\'':
      flush_until(pos-1);
      buffer << "&apos;";
      last_pos_writen = pos;
      break;
    case '<':
      flush_until(pos-1);
      buffer << "&lt;";
      last_pos_writen = pos;
      break;
    case '>':
      flush_until(pos-1);
      buffer << "&gt;";
      last_pos_writen = pos;
      break;
    default:
      // buffer << data[pos];    
      break;
    }
  }

  flush_until(data.size()-1);

}


template <typename B> void escape_html_entities(B& buffer, const std::string& data) {

  for (size_t pos = 0; pos != data.size(); ++pos) {
    switch (data[pos]) {
    case '&':
      buffer << "&amp;";
      break;
    case '\"':
      buffer << "&quot;";
      break;
    case '\'':
      buffer << "&apos;";
      break;
    case '<':
      buffer << "&lt;";
      break;
    case '>':
      buffer << "&gt;";
      break;
    default:
      buffer << data[pos];    
      break;
    }
  }

}

#define PGSQL
//#define BENCH_MYSQL

#ifdef BENCH_MYSQL
auto sql_db =
    mysql_database(s::host = "127.0.0.1", s::database = "mysql_test", s::user = "root",
                            s::password = "lithium_test", s::port = 14550, s::charset = "utf8");

#else
auto sql_db = pgsql_database(s::host = "127.0.0.1", s::database = "postgres", s::user = "postgres",
                             s::password = "lithium_test", s::port = 32768, s::charset = "utf8");

#endif

auto fortunes =
    sql_orm_schema(sql_db, "fortune")
        .fields(s::id(s::auto_increment, s::primary_key) = int(), s::message = std::string());

auto random_numbers =
    sql_orm_schema(sql_db, "world")
        .fields(s::id(s::auto_increment, s::primary_key) = int(), s::randomNumber = int());

void set_max_sql_connections_per_thread(int max) {
  sql_db.max_async_connections_per_thread_ = max;
}

int nprocs =  std::thread::hardware_concurrency();

#ifdef BENCH_MYSQL
int nthread = 4;
// int db_nconn = 4*nprocs;
// int queries_nconn = 2*nprocs;
// int fortunes_nconn = 4*nprocs;
// int updates_nconn = 100*nprocs;

// int db_nconn = 90;
// int queries_nconn = 40;
// int fortunes_nconn = 4000;
// int updates_nconn = 30;

int db_nconn = 99989 / 4;
int queries_nconn = 99989 / 4;
int fortunes_nconn = 99989 / 4;
int updates_nconn = 99989 / 4;

// int nthread = nprocs;
// int db_nconn = 4;
// int queries_nconn = 2;
// int fortunes_nconn = 4;
// int updates_nconn = 1;
#else
// int nthread = 1;
// int db_nconn = 4*nprocs;
// int queries_nconn = 4*nprocs;
// int fortunes_nconn = 2*nprocs;
// int updates_nconn = 3*nprocs;

int db_nconn = 4;
int queries_nconn = 4;
int fortunes_nconn = 2;
int updates_nconn = 3;

int nthread = nprocs;
// int nthread = 1;
// int db_nconn = 90 / 4;
// int queries_nconn = 90 / 4;
// int fortunes_nconn = 90 / 4;
// int updates_nconn = 90 / 4;
#endif


// thread_local lru_cache<int, decltype(mmm(s::id = int(), s::randomNumber = int()))> world_cache(10000);
template <typename T>
struct cache {

  void insert(T o) { 
    buffer.push_back(o);
  }

  std::vector<const T*> get_array(const std::vector<int>& ids) {
    std::vector<const T*> res;
    for (int i = 0; i < ids.size(); i++) res.push_back(&buffer[ids[i]]);
    return res;
  }

  std::vector<T> buffer;
};

cache<decltype(mmm(s::id = int(), s::randomNumber = int()))> world_cache;

auto make_api() {

  http_api my_api;

  my_api.get("/plaintext") = [&](http_request& request, http_response& response) {
    // response.set_header("Content-Type", "text/plain");
    response.write("Hello world!");
  };

  my_api.get("/json") = [&](http_request& request, http_response& response) {
    response.write_json(s::message = "Hello world!");
  };
  my_api.get("/db") = [&](http_request& request, http_response& response) {
    set_max_sql_connections_per_thread(db_nconn);
    // std::cout << "start" << std::endl;
    response.write_json(random_numbers.connect(request.fiber).find_one(s::id = 14).value());
    // std::cout << "end" << std::endl;
  };

  my_api.get("/queries") = [&](http_request& request, http_response& response) {
    set_max_sql_connections_per_thread(queries_nconn);
    std::string N_str = request.get_parameters(s::N = std::optional<std::string>()).N.value_or("1");
    // std::cout << N_str << std::endl;
    int N = atoi(N_str.c_str());

    N = std::max(1, std::min(N, 500));

    std::vector<decltype(random_numbers.all_fields())> numbers(N);
    {
      auto c = random_numbers.connect(request.fiber);
      // auto& raw_c = c.backend_connection();
      // raw_c("START TRANSACTION");
      // auto stm = c.backend_connection().prepare("SELECT randomNumber from World where id=$1");
      for (int i = 0; i < N; i++)
        numbers[i] = c.find_one(s::id = 1 + rand() % 99).value();
      // numbers[i] = stm(1 + rand() % 99).read<std::remove_reference_t<decltype(numbers[i])>>();
    // {
    //   numbers[i].id = 1 + rand() % 99;
    //     numbers[i].randomNumber = stm(1 + rand() % 99).read<int>();
    // }
      // raw_c("COMMIT");
    }

    response.write_json(numbers);
  };


  random_numbers.connect().forall([&] (const auto& number) {
    world_cache.insert(metamap_clone(number));
  });

  my_api.get("/cached-worlds") = [&](http_request& request, http_response& response) {
    sql_db.max_async_connections_per_thread_ = queries_nconn;
    std::string N_str = request.get_parameters(s::N = std::optional<std::string>()).N.value_or("1");
    int N = atoi(N_str.c_str());
    
    N = std::max(1, std::min(N, 500));
    
    // if (world_cache.size() == 0)
    //   random_numbers.connect(request.fiber).forall([&] (const auto& number) {
    //     world_cache(number.id, [&] { return metamap_clone(number); });
    //   });

    // std::vector<decltype(random_numbers.all_fields())> numbers(N);
    // for (int i = 0; i < N; i++)
    // {
    //   int id = 1 + rand() % 10000;
    //   numbers[i] = world_cache(id);
    // }


    std::vector<int> ids(N);
    for (int i = 0; i < N; i++)
      ids[i] = 1 + rand() % 10000;
    response.write_json(world_cache.get_array(ids));

    // response.write_json(numbers);
  };

  my_api.get("/updates") = [&](http_request& request, http_response& response) {
    // try {
    // std::cout << "up" << std::endl;
    set_max_sql_connections_per_thread(updates_nconn);
    std::string N_str = request.get_parameters(s::N = std::optional<std::string>()).N.value_or("1");
    int N = atoi(N_str.c_str());
    N = std::max(1, std::min(N, 500));
    std::vector<decltype(random_numbers.all_fields())> numbers(N);

    {
      auto c = random_numbers.connect(request.fiber);
      auto& raw_c = c.backend_connection();

      {
        // auto c2 = random_numbers.connect(request.fiber);
        // auto& raw_c2 = c2.backend_connection();

        // raw_c("START TRANSACTION ISOLATION LEVEL REPEATABLE READ");
        // #ifdef BENCH_MYSQL
        // raw_c("START TRANSACTION");
        // #endif

        for (int i = 0; i < N; i++) {
          numbers[i] = c.find_one(s::id = 1 + rand() % 9999).value();
          // numbers[i].id = 1 + rand() % 9999;
          // numbers[i].randomNumber = raw_c.cached_statement([] { return "select randomNumber from world where id=?"; })(numbers[i].id).read<int>();
          numbers[i].randomNumber = 1 + rand() % 9999;
        }
        // raw_c("COMMIT");
      }

      std::sort(numbers.begin(), numbers.end(), [](auto a, auto b) { return a.id < b.id; });

#ifdef BENCH_MYSQL
      for (int i = 0; i < N; i++)
        c.update(numbers[i]);
#else
      c.bulk_update(numbers);
      // raw_c.cached_statement([N] {
      //     std::ostringstream ss;
      //     ss << "UPDATE World SET randomNumber=tmp.randomNumber FROM (VALUES ";
      //     for (int i = 0; i < N; i++)
      //       ss << "($" << i*2+1 << "::int, $" << i*2+2 << "::int) "<< (i == N-1 ? "": ",");
      //     ss << " ) AS tmp(id, randomNumber) WHERE tmp.id = World.id";
      //     return ss.str();
      // }, N)(numbers);

#endif
      // #ifdef BENCH_MYSQL
      // raw_c("COMMIT");
      // #endif
    }

    // std::cout << raw_c.prepare("select randomNumber from World where
    // id=$1")(numbers[0].id).read<int>() << " " << numbers[0].randomNumber << std::endl;
    response.write_json(numbers);
  };

  my_api.get("/fortunes") = [&](http_request& request, http_response& response) {
    set_max_sql_connections_per_thread(fortunes_nconn);
    typedef decltype(fortunes.all_fields()) fortune;


    // auto comp = [](const fortune& a, const fortune& b) { return a.message < b.message; };
    // std::vector<fortune> table = { fortune(0, std::string("Additional fortune added at request time.")) };
    // auto c = fortunes.connect(request.fiber);
    // c.forall([&](auto f) { table.emplace_back(f); });
    // table.emplace_back(0, std::string("Additional fortune added at request time."));
    // std::sort(table.begin(), table.end(),
    //           [](const fortune& a, const fortune& b) { return a.message < b.message; });

    // auto comp = [](const fortune& a, const fortune& b) { return a.message < b.message || (a.message == b.message && a.id < b.id); };
    // auto comp = [](const fortune& a, const fortune& b) { return a.message < b.message; };
    // auto table = std::set<fortune, decltype(comp)>(comp);
    // auto c = fortunes.connect(request.fiber);
    // c.forall([&](auto f) { table.insert(f); });
    // table.emplace(0, std::string("Additional fortune added at request time."));


    auto comp = [](const fortune& a, const fortune& b) { return a.message < b.message; };
    std::vector<fortune> table = { fortune(0, std::string("Additional fortune added at request time.")) };

    {
      auto c = fortunes.connect(request.fiber);
      c.forall([&] (const auto& f) { table.emplace_back(metamap_clone(f)); });
    }    
    // std::sort(table.begin(), table.end(),
    //           [] (const fortune& a, const fortune& b) { return a.message < b.message; });

    // c.forall([&](const auto& f) { insert_sorted(table, metamap_clone(f), comp); });

    char b[100000];
    li::output_buffer ss(b, sizeof(b));
     ss << "<!DOCTYPE "
          "html><html><head><title>Fortunes</title></head><body><table><tr><th>id</th><th>message</"
          "th></tr>";
    for (auto& f : table) {
      ss << "<tr><td>" << f.id << "</td><td>";
      // ss << f.message;
      escape_html_entities(ss, f.message);
      ss << "</td></tr>";
    }
    ss << "</table></body></html>";

    response.set_header("Content-Type", "text/html; charset=utf-8");
    response.write(ss.to_string_view());
  };

  return my_api;
}

float tune_n_sql_connections(int& nc_to_tune, std::string http_req, int port, int max, int nprocs) {
#if __linux__ 
  auto sockets = http_benchmark_connect(512, port);

  std::cout << std::endl << "Benchmark " << http_req << std::endl;

  float max_req_per_s = 0;
  int best_nconn = 2;
  for (int nc : {1, 2, 4, 8, 32, 64, 128}) {
    if (nc * nprocs >= max)
      break;
    nc_to_tune = nc;
    float req_per_s = http_benchmark(sockets, 1, 5000, http_req);
    std::cout << nc << " -> " << req_per_s << " req/s." << std::endl;
    if (req_per_s > max_req_per_s) {
      max_req_per_s = req_per_s;
      best_nconn = nc;
    }
  }
  http_benchmark_close(sockets);

  std::cout << "best: " << best_nconn << " (" << max_req_per_s << " req/s)." << std::endl;
  nc_to_tune = best_nconn;
  return best_nconn;
  #endif
  return 1;
}
int main(int argc, char* argv[]) {

  if (argc == 3) { // init.

    auto c = random_numbers.connect();
    c.backend_connection()("START TRANSACTION");
    c.drop_table_if_exists().create_table_if_not_exists();
    for (int i = 0; i < 10000; i++)
      c.insert(s::randomNumber = i);
    c.backend_connection()("COMMIT");
    auto f = fortunes.connect();
    f.drop_table_if_exists().create_table_if_not_exists();
    for (int i = 0; i < 100; i++)
      f.insert(s::message = std::string("testmessagetestmessagetestmessagetestmessagetestmessagetestmessage") + boost::lexical_cast<std::string>(i));
  }

  int port = atoi(argv[1]);

  auto my_api = make_api();

  // std::thread server_thread([&] {
  //   http_serve(my_api, port, s::nthreads = 4);
  // });
  // usleep(1e5);

  // std::cout << http_benchmark(512, 1, 5000, port, "GET /db HTTP/1.1\r\n\r\n") << std::endl;
  // std::cout << http_benchmark(512, 1, 5000, port, "GET /db HTTP/1.1\r\n\r\n") << std::endl;
  // std::cout << http_benchmark(512, 1, 5000, port, "GET /db HTTP/1.1\r\n\r\n") << std::endl;
  // std::cout << http_benchmark(512, 1, 5000, port, "GET /db HTTP/1.1\r\n\r\n") << std::endl;
  // server_thread.join();

  // return 0;

#ifdef BENCH_MYSQL
  // int sql_max_connection =
  //     sql_db.connect()("SELECT @@GLOBAL.max_connections;").template read<int>() - 10;
  // std::cout << "mysql max connection " << sql_max_connection << std::endl;
  // int port = 12667;
  //li::max_mysql_connections_per_thread = (sql_max_connection / nthread);

#else
  // int sql_max_connection =
  //     atoi(sql_db.connect()("SHOW max_connections;").template read<std::string>().c_str()) - 10;
  // std::cout << "sql max connection " << sql_max_connection << std::endl;
  // int port = 12543;
  //li::max_pgsql_connections_per_thread = (sql_max_connection / nthread);
#endif

  // int tunning_port = port+1;
  // std::thread server_thread([&] {
  //   http_serve(my_api, tunning_port, s::nthreads = nthread);
  // });
  // usleep(3e5);

  // // db_nconn = tune_n_sql_connections("GET /db HTTP/1.1\r\n\r\n", port, sql_max_connection /
  // nthread);
  // // queries_nconn = tune_n_sql_connections("GET /queries?N=20 HTTP/1.1\r\n\r\n", port,
  // sql_max_connection / nthread);
  // // fortunes_nconn = tune_n_sql_connections("GET /fortunes HTTP/1.1\r\n\r\n", port,
  // sql_max_connection / nthread); update_nconn = tune_n_sql_connections(update_nconn, "GET
  // /updates?N=20 HTTP/1.1\r\n\r\n", tunning_port, sql_max_connection, nthread);
  // li::quit_signal_catched = true;
  // server_thread.join();
  // li::quit_signal_catched = false;
  std::cout << "Starting server on port " << port << std::endl;
  http_serve(my_api, port, s::nthreads = nthread);
  // server_thread.join();

  return 0;
}
