cmake_minimum_required(VERSION 3.16)

file(GLOB_RECURSE networked_extras_source ${CMAKE_CURRENT_LIST_DIR}/source/*.cpp)
add_library(networked_extras ${networked_extras_source})

target_include_directories(networked_extras PUBLIC include PRIVATE source)

