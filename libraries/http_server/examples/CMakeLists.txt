cmake_minimum_required(VERSION 3.0)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                      ${CMAKE_SOURCE_DIR}/../cmake)

enable_testing()

find_package(MYSQL REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(CURL REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(${SQLite3_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS} ${MYSQL_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR})

include_directories(${OPENSSL_INCLUDE_DIR})

set(LIBS ${SQLite3_LIBRARIES} ${CURL_LIBRARIES} ${MYSQL_LIBRARY}
         ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_LIBRARIES})
message(${Boost_LIBRARIES})
li_add_executable(blog blog.cc)
target_link_libraries(blog ${LIBS})
