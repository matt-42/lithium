li::http_client
===============================

*Tested compilers: Linux: G++ 9, Clang++ 9, Macos: Clang 11, Windows: MSVC 19*

`li::http_client` is an easy to use http client built around
the libcurl library.

Installation
============================

Either install the single header library
```
wget https://github.com/matt-42/lithium/blob/master/single_headers/li_http_client.hh
```
and include the **li_http_client.hh** header. In this case the namespace of the library is **li_http_client**.

Or install the lithium project: https://github.com/matt-42/lithium
and include the **li/http_client/http_client.hh** header. In this case the namespace of the library is **li**.

You will also have to install and link your program with the libcurl library.

Tutorial
========================

```c++

// Simple GET request:
auto res = http_get("http://www.google.com");
// returns an object with a status and a body member.
std::cout << res.status << std::endl;
std::cout << res.body << std::endl;

// http_post, http_put, http_delete are also avalable.

// GET and POST Parameters.
auto res = http_post("http://my_api.com/hello", 
                     s::get_parameters = mmm(s::id = 42), 
                     s::post_parameters = mmm(s::id = 42));

// Access to headers.
auto res = http_get("http://my_api.com/hello", s::fetch_headers);
// returns an object with a status, body AND headers member.
for (auto it : res.headers) {
  std::cout << it.first << ":::" << it.second  << std::endl;
}
```
