#pragma once

#include <iod/silicon/sql_orm.hh>
#include <iod/silicon/error.hh>

namespace iod {
  using namespace iod;
  
  template <typename ORM, typename... T>
  auto sql_crud(ORM& orm, T&&... _opts)
  {    
    typedef typename ORM::connection_type ORMI;
    typedef typename ORMI::object_type O; // O without primary keys for create procedure.
    typedef typename ORMI::PKS PKS; // Object with only the primary keys for the delete procedure.

    typedef sql_orm_internals::remove_read_only_fields_t<O> update_type;
    typedef sql_orm_internals::remove_auto_increment_t<update_type> insert_type;
    
    auto call_callback = [] (auto& f, O& o, auto& deps)
    {
      return sql_crud_call_callback(f, o, deps.deps);
      //return iod::apply(f, o, deps.deps, o, di_call(f, o, deps.deps, di_call);
    };

    auto opts = D(_opts...);
    auto read_access = opts.get(s::read_access, [] () { return true; });
    auto write_access = opts.get(s::write_access, [] () { return true; });
    auto validate = opts.get(s::validate, [] () { return true; });
    auto on_create_success = opts.get(s::on_create_success, [] () {});
    auto on_destroy_success = opts.get(s::on_destroy_success, [] () {});
    auto on_update_success = opts.get(s::on_update_success, [] () {});

    auto before_update = opts.get(s::before_update, [] () {});
    auto before_create = opts.get(s::before_create, [] () {});
    
    //auto before_destroy = opts.get(s::before_destroy, [] () {});

    api<http_request, http_response> api;

    
    api(GET, "get_by_id") = [=, &orm] (http_request& request,
				      http_response& response)
    {
      int id = request.get_parameters(s::id = int()).id;
      O o;
      if (!orm.get_connection().find_by_id(id, o))
        throw http_error::not_found("Object not found");
      if (read_access(request, response, o))
        return o;
      else
        throw http_error::unauthorized("Not enough priviledges to access this object");
    };

    
    api(POST, "create") =
      [=, &orm] (http_request& request,
	   http_response& response)
    {
      auto obj = request.post_parameters(insert_type());
      O o;
      o = obj;
      before_create(request, response, o);
      if (validate(request, response, o))
      {
        int new_id = orm.get_connection().insert(o);
        on_create_success(request, response, o);
        return D(s::id = new_id);
      }
      else
        throw http_error::unauthorized("Not enough priviledges to edit this object");
    };

    api(POST, "update") = [=, &orm] (http_request& request,
				    http_response& response)
    {
      auto obj = request.post_parameters(update_type());
      O o;
      if (!orm.get_connection().find_by_id(obj.id, o))
        throw http_error::not_found("Object with id ", obj.id, " not found");

      if (!write_access(request, response, o))
        throw http_error::unauthorized("Not enough priviledges to edit this object");

      // Update
      o = obj;      
      if (!validate(request, response, o))
        throw http_error::bad_request("Invalid update parameters");
      
      before_update(request, response, o);
      orm.update(o);
      on_update_success(request, response, o);
    };

    api(POST, "destroy") = [=, &orm] (http_request& request,
				     http_response& response)
    {
      auto params = request.post_parameters(PKS());
      auto c = orm.get_connection();
      O o;
      if (!c.find_by_id(params.id, o))
        throw http_error::not_found("Delete: Object with id ", params.id, " not found");
      
      if (write_access(request, response, o))
      {
        c.destroy(params);
        on_destroy_success(request, response, o);
      }
      else
        throw http_error::unauthorized("Cannot delete this object");
    };
  }
  
  
}
