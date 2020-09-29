#pragma once

#include <li/json/json.hh>
#include <li/sql/sql_common.hh>
#include <li/sql/sql_orm.hh>

#include <li/http_backend/api.hh>
#include <li/http_backend/http_serve.hh>
#include <li/http_backend/request.hh>
#include <li/http_backend/response.hh>

namespace li {
using http_api = api<http_request, http_response>;
}

#include <li/http_backend/hashmap_http_session.hh>
#include <li/http_backend/http_authentication.hh>
//#include <li/http_backend/mhd.hh>
#include <li/http_backend/serve_directory.hh>
#include <li/http_backend/sql_crud_api.hh>
#include <li/http_backend/sql_http_session.hh>
#include <li/http_backend/symbols.hh>
#include <li/http_backend/growing_output_buffer.hh>

#if __linux__
#include <li/http_backend/http_benchmark.hh>
#endif

#include <li/http_backend/lru_cache.hh>
