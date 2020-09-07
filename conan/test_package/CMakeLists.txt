cmake_minimum_required(VERSION 2.8.12)
project(PackageTest CXX)

set(CMAKE_CXX_STANDARD 17)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

#find_package(MYSQL REQUIRED)
#find_package(PostgreSQL REQUIRED)
#find_package(SQLite3 REQUIRED)
#include_directories(${SQLite3_INCLUDE_DIRS} ${MYSQL_INCLUDE_DIR})
#include_directories(${PostgreSQL_INCLUDE_DIRS})
#set(LIBS ${SQLite3_LIBRARIES} ${MYSQL_LIBRARY} ${PostgreSQL_LIBRARIES})

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

set(LIBS ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_LIBRARIES})

add_executable(example example.cpp)
target_link_libraries(example ${CONAN_LIBS})
