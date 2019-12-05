li::http_client
===============================

`li::http_client` is an easy to use http client built around
the libcurl library.


# Tutorial

```c++
using namespace li_http_client;

// Simple GET request:
auto res = http_get("http://www.google.com");
// returns an object with a status and a body member.
std::cout << res.status << std::endl;
std::cout << res.body << std::endl;

// http_post, http_put, http_delete are also avalable.

// GET and POST Parameters.
auto res = http_post("http://my_api.com/update_test", 
                     s::get_parameters = mmm(s::id = 42), 
                     s::post_parameters = mmm(s::name = "John", s::age = 42));

// Access to headers.
auto res = http_get("http://my_api.com/hello", s::fetch_headers);
// returns an object with a status, body AND headers member.
for (auto it : res.headers) {
  std::cout << it.first << ":::" << it.second  << std::endl;
}
```

# Installation / Supported compilers

Everything explained here: https://github.com/matt-42/lithium#installation


# Authors

Matthieu Garrigues https://github.com/matt-42


# Donate

If you find this project helpful, please consider donating:
https://www.paypal.me/matthieugarrigues
