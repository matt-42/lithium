cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED context)
find_package(SQLite3 REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(${SQLite3_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR})

# Directory where lithium headers are located if not in the default include path.
include_directories($ENV{HOME}/local/include)

set(LIBS ${SQLite3_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_LIBRARIES})

add_custom_target(symbols_generation COMMAND li_symbol_generator ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(my_api my_api.cc)

target_precompile_headers(my_api
  PUBLIC
  <lithium_http_backend.hh>
  <lithium_sqlite.hh>
  )

add_dependencies(my_api symbols_generation)
target_link_libraries(my_api ${LIBS})
