cmake_minimum_required(VERSION 2.8)

project(li)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_BINARY_DIR}/single_headers) # docs use single headers.

function(add_sh_executable target_name)
  add_executable(${target_name} ${ARGN})
  target_link_libraries(${target_name} ${LIBS})
  add_dependencies(${target_name} single_headers symbols_generation)
endfunction(add_sh_executable)

#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -mtune=native -flto") 
add_definitions(-Wno-ignored-attributes) # to skip postgresql warnings.

add_sh_executable(sql sql.cc)
