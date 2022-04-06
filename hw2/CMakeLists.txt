cmake_minimum_required(VERSION 3.16)

file(GLOB_RECURSE networked_hw2_src ${CMAKE_CURRENT_LIST_DIR}/source/*.cpp)
message(${networked_hw2_src})

add_executable(networked_hw2_lobby ${networked_hw2_src} main_lobby.cpp)
add_executable(networked_hw2_client ${networked_hw2_src} main_client.cpp)
add_executable(networked_hw2_play_server ${networked_hw2_src} main_play_server.cpp)

target_include_directories(networked_hw2_lobby PUBLIC include PRIVATE source)
target_link_libraries(networked_hw2_lobby networked_extras)

target_include_directories(networked_hw2_client PUBLIC include PRIVATE source)
target_link_libraries(networked_hw2_client networked_extras)

target_include_directories(networked_hw2_play_server PUBLIC include PRIVATE source)
target_link_libraries(networked_hw2_play_server networked_extras)

CPMAddPackage(
        NAME enet
        GITHUB_REPOSITORY lsalzman/enet
        GIT_TAG v1.3.17
        )
target_link_libraries(networked_hw2_lobby enet)
target_include_directories(networked_hw2_lobby
        PUBLIC
        $<BUILD_INTERFACE:${enet_SOURCE_DIR}/include>)

target_link_libraries(networked_hw2_client enet)
target_include_directories(networked_hw2_client
        PUBLIC
        $<BUILD_INTERFACE:${enet_SOURCE_DIR}/include>)

target_link_libraries(networked_hw2_play_server enet)
target_include_directories(networked_hw2_play_server
        PUBLIC
        $<BUILD_INTERFACE:${enet_SOURCE_DIR}/include>)