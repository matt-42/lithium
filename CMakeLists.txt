cmake_minimum_required(VERSION 2.8)

project(iod)

add_definitions(-std=c++1z)

enable_testing()

include_directories(${CMAKE_BINARY_DIR}/include)

add_custom_target(prepare_include_dir ALL
    COMMAND sh ${CMAKE_SOURCE_DIR}/tools/prepare_include_directory.sh ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/include)
  
add_subdirectory(libraries/metamap)
add_subdirectory(libraries/metajson)
add_subdirectory(libraries/symbol)
add_subdirectory(libraries/di)
