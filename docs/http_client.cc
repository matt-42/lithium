
/*
http_client
===============================

## Header
```c++
#include <lithium_http_client.hh>
```

## Introduction
`li::http_client` is an easy to use blocking http client wrapping the libcurl library.
Lithium use it internally to test http servers. Thanks to Lithium's metamap and named parameters, it
simplifies a lot the writing of HTTP requests.

### Dependencies
- libcurl


## Tutorial

*/

#include <lithium_http_client.hh>

using namespace li;

int main() 
{

  // Simple GET request:
  auto res = http_get("http://www.google.com");
  // returns an object with a status and a body member.
  std::cout << res.status << std::endl;
  std::cout << res.body << std::endl;

  // http_post, http_put, http_delete are also available.

  // GET and POST Parameters.
  auto res = http_post("http://my_api.com/update_test", 
                      s::get_parameters = mmm(s::id = 42), 
                      s::post_parameters = mmm(s::name = "John", s::age = 42));

  // Access to headers.
  auto res = http_get("http://my_api.com/hello", s::fetch_headers);
  // returns an object with a status, body AND headers member.
  for (auto pair : res.headers) {
    std::cout << pair.first << ":::" << pair.second  << std::endl;
  }

}
/*
## Reference

```c++
auto result = http_[get/post/put/delete](std::string url, options...)
```

Send a get/post/put/delete HTTP request at the provided url. The return value contains the following fields:
- `int result.status`: The status of the HTTP response
- `string result.body`: The body of the HTTP response
- `map<string, string> result.headers`: Only if option `s::fetch_headers` is provided. The headers of the HTTP response.


Available options:

  - `s::get_parameters` : a metamap of the GET parameters. 
  - `s::post_parameters` : a metamap of the POST parameters.
  - `s::fetch_headers` : add the headers field to the return value.
  - `s::disable_check_certificate`: disable SSL certificate check.
  - `s::json_encoded` : JSON encode the POST parameters (default: url encode) 
*/


  
  