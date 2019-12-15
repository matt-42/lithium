cmake_minimum_required(VERSION 2.8)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                      ${CMAKE_SOURCE_DIR}/../cmake)

enable_testing()

find_package(MYSQL REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(CURL REQUIRED)
find_package(microhttpd REQUIRED)
find_package(Threads REQUIRED)

include_directories(${SQLite3_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS} ${MICROHTTPD_INCLUDE_DIRS} ${MYSQL_INCLUDE_DIR})

set(CMAKE_VERBOSE_MAKEFILE ON)
if (OPENSSL_FOUND)
    add_definitions(-DBOOST_NETWORK_ENABLE_HTTPS)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

if (OPENSSL_FOUND)
  include_directories(${OPENSSL_INCLUDE_DIR})
endif (OPENSSL_FOUND)

set(LIBS ${MICROHTTPD_LIBRARIES} ${SQLite3_LIBRARIES} ${CURL_LIBRARIES} ${MYSQL_LIBRARY} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
message(${Boost_LIBRARIES})
li_add_executable(blog blog.cc)
target_link_libraries(blog ${LIBS})
