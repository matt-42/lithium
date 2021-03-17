cmake_minimum_required(VERSION 3.0)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                      ${CMAKE_SOURCE_DIR}/../cmake)

enable_testing()

add_definitions(-DLITHIUM_SERVER_NAME=l)

li_add_executable(bench bench.cc)
target_link_libraries(bench ${LIBS})


if (NOT APPLE)
  li_add_executable(bench_hello_world hello_world.cc)
  target_link_libraries(bench_hello_world ${LIBS})
endif()