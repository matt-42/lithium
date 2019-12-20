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

int main(int argc, char* argv[]) {

  // auto sql_db =
      // mysql_database(s::host = "127.0.0.1", s::database = "silicon_test", s::user = "root",
      //                s::password = "sl_test_password", s::port = 14550, s::charset = "utf8");

  auto sql_db = pgsql_database(s::host = "localhost", s::database = "postgres", s::user = "postgres",
                            s::password = "lithium_test", s::port = 32768, s::charset = "utf8");

  auto fortunes = sql_orm_schema(sql_db, "Fortune").fields(
    s::id(s::auto_increment, s::primary_key) = int(),
    s::message = std::string());

  auto random_numbers = sql_orm_schema(sql_db, "World").fields(
    s::id(s::auto_increment, s::primary_key) = int(),
    s::randomNumber = int());

    { // init.

      auto c = random_numbers.connect();
      c.drop_table_if_exists().create_table_if_not_exists();
      for (int i = 0; i < 100; i++)
        c.insert(s::randomNumber = i);
      auto f = fortunes.connect();
      f.drop_table_if_exists().create_table_if_not_exists();
      for (int i = 0; i < 10; i++)
        f.insert(s::message = "testmessagetestmessagetestmessagetestmessagetestmessagetestmessage");
    }
  http_api my_api;

  my_api.get("/plaintext") = [&](http_request& request, http_response& response) {
    response.set_header("Content-Type", "text/plain");
    response.write("Hello world!");
  };

  my_api.get("/json") = [&](http_request& request, http_response& response) {
    response.write_json(s::message = "Hello world!");
  };
  my_api.get("/db") = [&](http_request& request, http_response& response) {
    //std::cout << "start" << std::endl;
    response.write_json(random_numbers.connect(request.yield).find_one(s::id = 14).value());
    //std::cout << "end" << std::endl;
  };

  my_api.get("/queries") = [&](http_request& request, http_response& response) {
    std::string N_str = request.get_parameters(s::N = std::optional<std::string>()).N.value_or("1");
    //std::cout << N_str << std::endl;
    int N = atoi(N_str.c_str());
    
    N = std::max(1, std::min(N, 500));
    
    auto c = random_numbers.connect(request.yield);
    std::vector<decltype(random_numbers.all_fields())> numbers(N);
    for (int i = 0; i < N; i++)
      numbers[i] = c.find_one(s::id = 1 + rand() % 99).value();

    response.write_json(numbers);
  };

  my_api.get("/updates") = [&](http_request& request, http_response& response) {
    //std::cout << "start" << std::endl;
    std::string N_str = request.get_parameters(s::N = std::optional<std::string>()).N.value_or("1");
    //std::cout << N_str << std::endl;
    int N = atoi(N_str.c_str());
    N = std::max(1, std::min(N, 500));
    
    auto c = random_numbers.connect(request.yield);
    std::vector<decltype(random_numbers.all_fields())> numbers(N);
    for (int i = 0; i < N; i++)
    {
      numbers[i] = c.find_one(s::id = 1 + rand() % 99).value();
      numbers[i].randomNumber = 1 + rand() % 99;
      c.update(numbers[i]);
    }
    //std::cout << "end" << std::endl;
    response.write_json(numbers);
  };

  my_api.get("/fortunes") = [&](http_request& request, http_response& response) {

    typedef decltype(fortunes.all_fields()) fortune;

    std::vector<fortune> table;

    auto c = fortunes.connect(request.yield);
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
  
  // int mysql_max_connection = sql_db.connect()("SELECT @@GLOBAL.max_connections;").read<int>() * 0.75;
  // std::cout << "mysql max connection " << mysql_max_connection << std::endl;
  // int port = atoi(argv[1]);
  // //int port = 12667;
  // int nthread = 4;
  // li::max_mysql_connections_per_thread = (mysql_max_connection / nthread);

  int mysql_max_connection = atoi(sql_db.connect()("SHOW max_connections;").read<std::string>().c_str()) * 0.75;
  std::cout << "sql max connection " << mysql_max_connection << std::endl;
  int port = atoi(argv[1]);
  int nthread = 4;
  li::max_pgsql_connections_per_thread = (mysql_max_connection / nthread);


  http_serve(my_api, port, s::nthreads = nthread); 
  
  return 0;

}
