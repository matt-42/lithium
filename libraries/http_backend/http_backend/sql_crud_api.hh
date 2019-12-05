#pragma once

#include <li/json/json.hh>
#include <li/http_backend/error.hh>
#include <li/sql/sql_orm.hh>


namespace li {
  
template <typename A, typename B, typename C> auto sql_crud_api(sql_orm_schema<A, B, C>& orm_schema) {

  api<http_request, http_response> api;

  api(POST, "/find_by_id") = [&](http_request& request, http_response& response) {
    auto params = request.post_parameters(s::id = int());
    if (auto obj = orm_schema.connect().find_one(s::id = params.id, request, response))
      response.write(json_encode(obj));
    else
      throw http_error::not_found(orm_schema.table_name(), " with id ", params.id, " does not exist.");
  };

  api(POST, "/create") = [&](http_request& request, http_response& response) {
    auto insert_fields = orm_schema.all_fields_except_computed();
    auto obj = request.post_parameters(insert_fields);
    long long int id = orm_schema.connect().insert(obj, request, response);
    response.write(json_encode(s::id = id));
  };

  api(POST, "/update") = [&](http_request& request, http_response& response) {
    auto obj = request.post_parameters(orm_schema.all_fields());
    orm_schema.connect().update(obj, request, response);
  };

  api(POST, "/remove") = [&](http_request& request, http_response& response) {
    auto obj = request.post_parameters(orm_schema.primary_key());
    orm_schema.connect().remove(obj, request, response);
  };

  return api;
}

} // namespace li
