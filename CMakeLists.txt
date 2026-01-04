cmake_minimum_required(VERSION 3.14)
project(CRayEngine VERSION 0.002)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# --- Fetch Raylib ---
include(FetchContent)
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.0.tar.gz
)
FetchContent_MakeAvailable(raylib)

# --- Sources ---
set(SOURCES
    src/main.c

    # Core Engine
    src/engine/core/engine.c
    src/engine/core/ui_helper.c
    src/engine/core/logger.c
    src/engine/core/scene_manager.c
    src/engine/core/input.c

    # Game
    src/game/game.c
    src/game/entities/snake.c
    src/game/entities/food.c
    src/game/game_scenes.c
    src/game/menu.c
    src/game/game_over.c
)

# --- Executable ---
add_executable(${PROJECT_NAME} ${SOURCES})

# --- Include Directories ---
target_include_directories(${PROJECT_NAME} PRIVATE src include)

# --- Linking ---
target_link_libraries(${PROJECT_NAME} raylib m)

# --- Assets ---
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})
