cmake_minimum_required(VERSION 3.11)
project(multiplayer C)

set(CMAKE_C_STANDARD 11)

include(FetchContent)
FetchContent_Declare(
        libuv
        GIT_REPOSITORY https://github.com/libuv/libuv.git
        GIT_TAG        v1.51.0
)
FetchContent_MakeAvailable(libuv)
add_executable(multiplayer "main.c"
        snake.h
        uthash.h)
target_link_libraries(multiplayer PRIVATE uv ${CMAKE_DL_LIBS})