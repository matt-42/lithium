cmake_minimum_required(VERSION 3.0)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                      ${CMAKE_SOURCE_DIR}/../cmake)

enable_testing()

li_add_executable(blog blog.cc)
li_add_executable(hello_world_example hello_world.cc)
