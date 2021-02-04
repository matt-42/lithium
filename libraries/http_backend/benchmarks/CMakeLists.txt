cmake_minimum_required(VERSION 3.0)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                      ${CMAKE_SOURCE_DIR}/../cmake)

enable_testing()

find_package(MYSQL REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(CURL REQUIRED)
find_package(Threads REQUIRED)
# find_package(PostgreSQL REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(${SQLite3_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS} ${MYSQL_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR})

# include_directories(${PostgreSQL_INCLUDE_DIRS})
include_directories($ENV{HOME}/local/libpq-pipelining/include)

link_directories($ENV{HOME}/local/libpq-pipelining/lib)

#SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -lgcov --coverage -fprofile-generate")
#add_definitions(-fprofile-generate)
#SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fprofile-use")
#add_definitions(-fprofile-use)

#add_definitions(-fprofile-instr-generate=/tmp/profile.bench)
#SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fprofile-instr-generate=/tmp/profile.bench")

#llvm-profdata merge -output=/tmp/profile.pgo  /tmp/profile.bench

#add_definitions(-fprofile-instr-use=/tmp/profile.pgo)
#SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fprofile-instr-use=/tmp/profile.pgo")

#add_definitions(-gline-tables-only)
#add_definitions(-fprofile-sample-use=/tmp/code.prof) 
#SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fprofile-sample-use=/tmp/code.prof")
# add_definitions(-fno-omit-frame-pointer -pg)

set(CMAKE_VERBOSE_MAKEFILE ON)
include_directories(${OPENSSL_INCLUDE_DIR})

set(LIBS ${SQLite3_LIBRARIES} ${CURL_LIBRARIES} 
          ${MYSQL_LIBRARY} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
          # ${PostgreSQL_LIBRARIES} 
          pq
          ${OPENSSL_LIBRARIES})

li_add_executable(bench bench.cc)
target_link_libraries(bench ${LIBS})