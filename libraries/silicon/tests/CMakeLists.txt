cmake_minimum_required(VERSION 2.8)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                      ${CMAKE_SOURCE_DIR}/../cmake)

enable_testing()

find_package(SQLite3 REQUIRED)
find_package(CURL REQUIRED)
find_package(microhttpd REQUIRED)

include_directories(${SQLite3_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS} ${MICROHTTPD_INCLUDE_DIRS})
#find_package( OpenSSL )
#find_package( Threads )
#find_package( Boost COMPONENTS system thread date_time program_options filesystem chrono)

set(CMAKE_VERBOSE_MAKEFILE ON)
if (OPENSSL_FOUND)
    add_definitions(-DBOOST_NETWORK_ENABLE_HTTPS)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

if (OPENSSL_FOUND)
  include_directories(${OPENSSL_INCLUDE_DIR})
endif (OPENSSL_FOUND)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g -O0")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage") # enabling coverage

set(LIBS ${MICROHTTPD_LIBRARIES} ${SQLite3_LIBRARIES} ${CURL_LIBRARIES})

add_executable(url_parameter url_parameter.cc)
target_link_libraries(url_parameter ${LIBS})
add_test(url_parameter url_parameter)


add_executable(url_decode url_decode.cc)
target_link_libraries(url_decode ${LIBS})
add_test(url_decode url_decode)

add_executable(sql_orm sql_orm.cc)
target_link_libraries(sql_orm ${LIBS})
add_test(sql_orm sql_orm)

add_executable(hello_world hello_world.cc)
target_link_libraries(hello_world ${LIBS})
add_test(hello_world hello_world)

add_executable(crud crud.cc)
target_link_libraries(crud ${LIBS})
add_test(crud crud)

add_executable(request_parameters request_parameters.cc)
target_link_libraries(request_parameters ${LIBS})
add_test(request_parameters request_parameters)

add_executable(sqlite_authentication sqlite_authentication.cc)
target_link_libraries(sqlite_authentication ${LIBS})
add_test(sqlite_authentication sqlite_authentication)
