cmake_minimum_required(VERSION 2.8)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                      ${CMAKE_SOURCE_DIR}/../cmake)

enable_testing()

find_package(MYSQL REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(CURL REQUIRED)
find_package(microhttpd REQUIRED)
find_package(Threads REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(${SQLite3_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS} ${MICROHTTPD_INCLUDE_DIRS} ${MYSQL_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR})

include_directories(${PostgreSQL_INCLUDE_DIRS})

#add_definitions(-fno-omit-frame-pointer -g)

set(CMAKE_VERBOSE_MAKEFILE ON)
add_definitions(-DBOOST_NETWORK_ENABLE_HTTPS)
include_directories(${OPENSSL_INCLUDE_DIR})

set(LIBS ${MICROHTTPD_LIBRARIES} ${SQLite3_LIBRARIES} ${CURL_LIBRARIES} 
          ${MYSQL_LIBRARY} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
          ${PostgreSQL_LIBRARIES} ${OPENSSL_LIBRARIES})

li_add_executable(bench bench.cc)
target_link_libraries(bench ${LIBS})