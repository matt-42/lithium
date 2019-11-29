cmake_minimum_required(VERSION 2.8)

project(http_client)
find_package(CURL REQUIRED)


enable_testing()

include_directories(${CURL_INCLUDE_DIRS})

add_executable(http_client_test tests/test.cc)
target_link_libraries(http_client_test  ${CURL_LIBRARIES})

add_test(http_client_test http_client_test)

install(DIRECTORY http_client DESTINATION include/iod
  FILES_MATCHING PATTERN "*.hh")
install(DIRECTORY http_client DESTINATION include/iod
  FILES_MATCHING PATTERN "*.hpp")
