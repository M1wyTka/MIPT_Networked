cmake_minimum_required(VERSION 3.16)

add_subdirectory(bgfx.cmake)

file(GLOB_RECURSE networked_hw3_src ${CMAKE_CURRENT_LIST_DIR}/source/*.cpp)
message(${networked_hw3_src})

add_executable(networked_hw3_client ${networked_hw3_src} main_client.cpp)
add_executable(networked_hw3_play_server ${networked_hw3_src} main_play_server.cpp)

target_include_directories(networked_hw3_client PUBLIC include PRIVATE source)
target_link_libraries(networked_hw3_client networked_extras)

target_include_directories(networked_hw3_play_server PUBLIC include PRIVATE source)
target_link_libraries(networked_hw3_play_server networked_extras)

CPMAddPackage(
        NAME enet
        GITHUB_REPOSITORY lsalzman/enet
        GIT_TAG v1.3.17
)

CPMAddPackage(
        NAME glfw
        GITHUB_REPOSITORY glfw/glfw
        GIT_TAG 3.3.7
)

target_link_libraries(networked_hw3_client enet)
target_include_directories(networked_hw3_client
        PUBLIC
        $<BUILD_INTERFACE:${enet_SOURCE_DIR}/include>)

target_link_libraries(networked_hw3_play_server enet)
target_include_directories(networked_hw3_play_server
        PUBLIC
        $<BUILD_INTERFACE:${enet_SOURCE_DIR}/include>)

if (glfw_ADDED)
    target_include_directories(networked_hw3_client PUBLIC
            $<BUILD_INTERFACE:${glfw_SOURCE_DIR}/include>)
endif()

target_include_directories(networked_hw3_client PUBLIC
        ${BGFX_DIR}/examples/common>)

target_include_directories(networked_hw3_client PUBLIC
        ${BX_DIR}/include>)

target_include_directories(networked_hw3_client PUBLIC
        ${BGFX_DIR}/include>)
