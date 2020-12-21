cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_STANDARD 17)

# Feel free to remove MYSQL, SQLite3 and/or PostgreSQL if you don't use them.

# You need the cmake modules MYSQL and SQLite3 in
# https://github.com/matt-42/lithium/tree/master/cmake
# for this to work
find_package(MYSQL REQUIRED)
find_package(SQLite3 REQUIRED)

find_package(CURL REQUIRED)
find_package(Threads REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED context)


include_directories(${SQLite3_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS} ${MYSQL_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} ${PostgreSQL_INCLUDE_DIRS})

set(LIBS ${SQLite3_LIBRARIES} ${CURL_LIBRARIES} 
          ${MYSQL_LIBRARY} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
          ${PostgreSQL_LIBRARIES} ${OPENSSL_LIBRARIES})

# Directory where lithium headers are located if not in the default include path.
include_directories($ENV{HOME}/local/include)

add_custom_target(symbols_generation COMMAND li_symbol_generator ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(my_api my_api.cc)
add_dependencies(my_api symbols_generation)
target_link_libraries(my_api ${LIBS})
