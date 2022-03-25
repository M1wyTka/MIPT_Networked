cmake_minimum_required(VERSION 3.16)

file(GLOB_RECURSE networked_server_src ${CMAKE_CURRENT_LIST_DIR}/source/*.cpp)
add_executable(networked_server ${networked_server_src} main.cpp)

target_include_directories(networked_server PUBLIC include PRIVATE source)
target_link_libraries(networked_server networked_extras)