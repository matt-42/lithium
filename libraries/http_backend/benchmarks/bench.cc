#include <li/http_backend/http_backend.hh>
#include <li/sql/mysql.hh>
#include <li/sql/pgsql.hh>


#include "symbols.hh"
using namespace li;

template <typename B>
void escape_html_entities(B& buffer, const std::string& data)
{
    for(size_t pos = 0; pos != data.size(); ++pos) {
        switch(data[pos]) {
            case '&':  buffer << "&amp;";       break;
            case '\"': buffer << "&quot;";      break;
            case '\'': buffer << "&apos;";      break;
            case '<':  buffer << "&lt;";        break;
            case '>':  buffer << "&gt;";        break;
            default:   buffer << data[pos]; break;
        }
    }
}

#define PGSQL
// #define BENCH_MYSQL
#ifdef BENCH_MYSQL
  auto sql_db =
      mysql_database(s::host = "127.0.0.1", s::database = "silicon_test", s::user = "root",
                     s::password = "sl_test_password", s::port = 14550, s::charset = "utf8");

#else
  auto sql_db = pgsql_database(s::host = "127.0.0.1", s::database = "postgres", s::user = "postgres",
                            s::password = "lithium_test", s::port = 32768, s::charset = "utf8");

#endif


  auto fortunes = sql_orm_schema(sql_db, "Fortune").fields(
    s::id(s::auto_increment, s::primary_key) = int(),
    s::message = std::string());

  auto random_numbers = sql_orm_schema(sql_db, "World").fields(
    s::id(s::auto_increment, s::primary_key) = int(),
    s::randomNumber = int());

void set_max_sql_connections_per_thread(int max)
{
// #ifdef BENCH_MYSQL
//   li::max_mysql_connections_per_thread = max;
// #elif PGSQL
  li::max_pgsql_connections_per_thread = max;
// #endif
}

int db_nconn = 64;
int queries_nconn = 64;
int fortunes_nconn = 64;
int update_nconn = 64;

auto make_api() {
  http_api my_api;

  my_api.get("/plaintext") = [&](http_request& request, http_response& response) {
    //response.set_header("Content-Type", "text/plain");
    response.write("Hello world!");
  };

  my_api.get("/json") = [&](http_request& request, http_response& response) {
    response.write_json(s::message = "Hello world!");
  };
  my_api.get("/db") = [&](http_request& request, http_response& response) {
    //std::cout << "start" << std::endl;
    response.write_json(random_numbers.connect(request.fiber).find_one(s::id = 14).value());
    //std::cout << "end" << std::endl;
  };

  my_api.get("/queries") = [&](http_request& request, http_response& response) {
    std::string N_str = request.get_parameters(s::N = std::optional<std::string>()).N.value_or("1");
    //std::cout << N_str << std::endl;
    int N = atoi(N_str.c_str());
    
    N = std::max(1, std::min(N, 500));
    
    auto c = random_numbers.connect(request.fiber);
    auto& raw_c = c.backend_connection();
    //raw_c("START TRANSACTION");
    std::vector<decltype(random_numbers.all_fields())> numbers(N);
    //auto stm = c.backend_connection().prepare("SELECT randomNumber from World where id=$1");
    for (int i = 0; i < N; i++)
      numbers[i] = c.find_one(s::id = 1 + rand() % 99).value();
      //numbers[i] = stm(1 + rand() % 99).read<std::remove_reference_t<decltype(numbers[i])>>();
      //numbers[i].randomNumber = stm(1 + rand() % 99).read<int>();
    //raw_c("COMMIT");

    response.write_json(numbers);
  };

  my_api.get("/updates") = [&](http_request& request, http_response& response) {
    // try {
    // std::cout << "up" << std::endl;  
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

        for (int i = 0; i < N; i++)
        {
          numbers[i] = c.find_one(s::id = 1 + rand() % 9999).value();
          // numbers[i].id = 1 + rand() % 9999;
          // numbers[i].randomNumber = raw_c2.cached_statement([] { return "select randomNumber from World where id=$1"; })(numbers[i].id).read<int>();
          numbers[i].randomNumber = 1 + rand() % 9999;
        }
        // raw_c("COMMIT");
      }

      std::sort(numbers.begin(), numbers.end(), [] (auto a, auto b) { return a.id < b.id; });


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

    // std::cout << raw_c.prepare("select randomNumber from World where id=$1")(numbers[0].id).read<int>() << " " << numbers[0].randomNumber << std::endl;   
    response.write_json(numbers);
  };

  my_api.get("/fortunes") = [&](http_request& request, http_response& response) {

    typedef decltype(fortunes.all_fields()) fortune;

    std::vector<fortune> table;

    auto c = fortunes.connect(request.fiber);
    c.forall([&] (auto f) { table.emplace_back(std::move(f)); });
    table.emplace_back(0, std::string("Additional fortune added at request time."));

    std::sort(table.begin(), table.end(),
              [] (const fortune& a, const fortune& b) { return a.message < b.message; });

    char b[100000];
    li::output_buffer ss(b, sizeof(b));
    ss << "<!DOCTYPE html><html><head><title>Fortunes</title></head><body><table><tr><th>id</th><th>message</th></tr>";
    for(auto& f : table)
    {
      ss << "<tr><td>" << f.id << "</td><td>";
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


  auto sockets = http_benchmark_connect(512, port);

  std::cout << std::endl << "Benchmark " << http_req << std::endl;

  float max_req_per_s = 0;
  int best_nconn = 2;
  for (int nc : {1, 2, 4, 8, 32, 64, 128})
  {
    if (nc*nprocs >= max) break;
    nc_to_tune = nc;
    float req_per_s = http_benchmark(sockets, 1, 5000, http_req);
    std::cout << nc << " -> " << req_per_s << " req/s." << std::endl;
    if (req_per_s > max_req_per_s)
    {
      max_req_per_s = req_per_s;
      best_nconn = nc;
    }
  }
  http_benchmark_close(sockets);

  std::cout << "best: " << best_nconn << " (" << max_req_per_s << " req/s)."<< std::endl;
  nc_to_tune = best_nconn;
  return best_nconn;
}
int main(int argc, char* argv[]) {


  if (argc == 3)
    { // init.

      auto c = random_numbers.connect();
      c.backend_connection()("START TRANSACTION");
      c.drop_table_if_exists().create_table_if_not_exists();
      for (int i = 0; i < 10000; i++)
        c.insert(s::randomNumber = i);
      c.backend_connection()("COMMIT");
      auto f = fortunes.connect();
      f.drop_table_if_exists().create_table_if_not_exists();
      for (int i = 0; i < 100; i++)
        f.insert(s::message = "testmessagetestmessagetestmessagetestmessagetestmessagetestmessage");
    }

  int port = atoi(argv[1]);

  auto my_api = make_api();

  int nthread = 4;
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
  int sql_max_connection = sql_db.connect()("SELECT @@GLOBAL.max_connections;").template read<int>() - 10;
  std::cout << "mysql max connection " << sql_max_connection << std::endl;
  int port = atoi(argv[1]);
  // int port = 12667;
  li::max_mysql_connections_per_thread = (sql_max_connection / nthread);

#else
  int sql_max_connection = atoi(sql_db.connect()("SHOW max_connections;").template read<std::string>().c_str()) - 10;
  std::cout << "sql max connection " << sql_max_connection << std::endl;
  // int port = 12543;  
  li::max_pgsql_connections_per_thread = (sql_max_connection / nthread);
#endif

  int tunning_port = port+1;
  std::thread server_thread([&] {
    http_serve(my_api, tunning_port, s::nthreads = nthread);
  });
  usleep(3e5);

  // // db_nconn = tune_n_sql_connections("GET /db HTTP/1.1\r\n\r\n", port, sql_max_connection / nthread);
  // // queries_nconn = tune_n_sql_connections("GET /queries?N=20 HTTP/1.1\r\n\r\n", port, sql_max_connection / nthread);
  // // fortunes_nconn = tune_n_sql_connections("GET /fortunes HTTP/1.1\r\n\r\n", port, sql_max_connection / nthread);
  update_nconn = tune_n_sql_connections(update_nconn, "GET /updates?N=20 HTTP/1.1\r\n\r\n", tunning_port, sql_max_connection, nthread);
  li::quit_signal_catched = true;
  server_thread.join();
  li::quit_signal_catched = false;

  http_serve(my_api, port, s::nthreads = nthread); 
  //server_thread.join();

  return 0;
}
