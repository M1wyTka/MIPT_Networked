cmake_minimum_required(VERSION 3.16)

file(GLOB_RECURSE networked_hw4_src ${CMAKE_CURRENT_LIST_DIR}/source/*.cpp)
message(${networked_hw4_src})

add_executable(networked_hw4_client ${networked_hw4_src} main_client.cpp)
add_executable(networked_hw4_play_server ${networked_hw4_src} main_play_server.cpp)

target_include_directories(networked_hw4_client PUBLIC include PRIVATE source)

target_include_directories(networked_hw4_play_server PUBLIC include PRIVATE source)

if(WIN32)
    target_link_libraries(networked_hw4_client wsock32 ws2_32)
    target_link_libraries(networked_hw4_play_server wsock32 ws2_32)
endif()

CPMAddPackage(
        NAME enet
        GITHUB_REPOSITORY lsalzman/enet
        GIT_TAG v1.3.17
)

CPMAddPackage(
        NAME allegro
        GITHUB_REPOSITORY liballeg/allegro5
        GIT_TAG 5.2.7.0
        OPTIONS
        # Ah, yes, I want my build process to take 10 minutes by default, thank you!
        "WANT_DOCS off"
        "WANT_EXAMPLES off"
        "WANT_DEMO off"
        "WANT_TESTS off"
        "WANT_VIDEO off"
        "WANT_PHYSFS off"
        "WANT_IMAGE_WEBP off"
        "WANT_AUDIO off"
)

if(allegro_ADDED)
    # allegro's authors have no idea how to write proper cmake files
    target_include_directories(allegro INTERFACE
            "${allegro_SOURCE_DIR}/include"
            "${allegro_BINARY_DIR}/include"
            )

    foreach(ADDON font image color primitives)
        target_include_directories("allegro_${ADDON}" INTERFACE
                "${allegro_SOURCE_DIR}/addons/${ADDON}"
                )
    endforeach()
endif()

target_link_libraries(networked_hw4_client allegro allegro_font allegro_primitives)
target_link_libraries(networked_hw4_play_server allegro allegro_font allegro_primitives)

target_link_libraries(networked_hw4_client enet)
target_include_directories(networked_hw4_client
        PUBLIC
        $<BUILD_INTERFACE:${enet_SOURCE_DIR}/include>)

target_link_libraries(networked_hw4_play_server enet)
target_include_directories(networked_hw4_play_server
        PUBLIC
        $<BUILD_INTERFACE:${enet_SOURCE_DIR}/include>)

