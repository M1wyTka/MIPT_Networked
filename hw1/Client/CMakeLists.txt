cmake_minimum_required(VERSION 3.16)

file(GLOB_RECURSE networked_hw1_client_src ${CMAKE_CURRENT_LIST_DIR}/source/*.cpp)
message(${networked_hw1_client_src})

add_executable(networked_hw1_client ${networked_hw1_client_src} main.cpp)

target_include_directories(networked_hw1_client PUBLIC include PRIVATE source)
target_link_libraries(networked_hw1_client networked_extras)

