cmake_minimum_required(VERSION 3.0)

project(http_client)
find_package(CURL REQUIRED)


enable_testing()

include_directories(${CURL_INCLUDE_DIRS})

li_add_executable(http_client_test tests/test.cc)
target_link_libraries(http_client_test  ${CURL_LIBRARIES})

#add_test(http_client_test http_client_test)

