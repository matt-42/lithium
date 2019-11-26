#include <refactor/api.hh>
#include <refactor/mhd.hh>

using namespace sl;

iod_define_symbol(cookie)

int main()
{
  auto database = sqlite_connection_factory("db.sqlite");

  api<http_request, http_response> my_api;

  my_api("/hello_world") =
    [&] (http_request& request, http_response& response) {

      std::string session_id = get_and_set_tracking_cookie(request, response, "silicon_tracker");
      auto sql = database.get_connection();
      auto sql_session = sql_session(sql, session_id);

      response.write("hello world " + session_id);
    };

  http_serve(my_api, 12345);
}
