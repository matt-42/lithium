cmake_minimum_required(VERSION 2.8)

project(iod)

add_definitions(-std=c++1z)

enable_testing()

include_directories(${CMAKE_BINARY_DIR}/include)

add_custom_target(prepare_include_dir ALL
    COMMAND sh ${CMAKE_SOURCE_DIR}/tools/prepare_include_directory.sh ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/include)
  
add_subdirectory(metamap)
add_subdirectory(metajson)
add_subdirectory(symbol)
