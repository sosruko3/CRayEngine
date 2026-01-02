cmake_minimum_required(VERSION 3.14)
project(SnakeGame C)

set(CMAKE_C_STANDARD 99)

# --- Fetch Raylib ---
include(FetchContent)
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.0.tar.gz
)
FetchContent_MakeAvailable(raylib)

# --- Sources ---
file(GLOB_RECURSE SOURCES "src/*.c")

# --- Executable ---
add_executable(${PROJECT_NAME} ${SOURCES})

# --- Include Directories ---
target_include_directories(${PROJECT_NAME} PRIVATE src include)

# --- Linking ---
target_link_libraries(${PROJECT_NAME} raylib m)

# --- Assets ---
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})
