#pragma once

#include <li/json/json.hh>
#include <li/sql/sql_common.hh>
#include <li/sql/sql_orm.hh>

#include <li/http_server/api.hh>
#include <li/http_server/http_serve.hh>
#include <li/http_server/request.hh>
#include <li/http_server/response.hh>

namespace li {
using http_api = api<http_request, http_response>;
}

#include <li/http_server/hashmap_http_session.hh>
#include <li/http_server/http_authentication.hh>
//#include <li/http_server/mhd.hh>
#include <li/http_server/serve_directory.hh>
#include <li/http_server/sql_crud_api.hh>
#include <li/http_server/sql_http_session.hh>
#include <li/http_server/symbols.hh>
#include <li/http_server/growing_output_buffer.hh>

#if __linux__
#include <li/http_server/http_benchmark.hh>
#endif

#include <li/http_server/lru_cache.hh>
