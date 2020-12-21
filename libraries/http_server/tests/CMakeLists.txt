cmake_minimum_required(VERSION 2.8)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                      ${CMAKE_SOURCE_DIR}/../cmake)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g -O0")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage") # enabling coverage


li_add_executable(url_parameter url_parameter.cc)
add_test(url_parameter url_parameter)

li_add_executable(url_decode url_decode.cc)
add_test(url_decode url_decode)

li_add_executable(hello_world hello_world.cc)
add_test(hello_world hello_world)

li_add_executable(crud crud.cc)
add_test(crud crud)

li_add_executable(lru_cache lru_cache.cc)
add_test(lru_cache lru_cache)

li_add_executable(request_parameters request_parameters.cc)
add_test(request_parameters request_parameters)

li_add_executable(sql_authentication sql_authentication.cc)
add_test(sql_authentication sql_authentication)

li_add_executable(http_authentication http_authentication.cc)
add_test(http_authentication http_authentication)

li_add_executable(hashmap_http_session hashmap_http_session.cc)
add_test(hashmap_http_session hashmap_http_session)

li_add_executable(serve_directory serve_directory.cc)
add_test(serve_directory serve_directory)

li_add_executable(https https.cc)
add_test(https https)

li_add_executable(benchmark_http benchmark_http.cc)
