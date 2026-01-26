cmake_minimum_required(VERSION 3.14)
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are not allowed. Please run 'cmake -B build' and 'cmake --build build'.")
endif()

project(CRayEngine VERSION 0.6)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# This finds the actual path to 'python3' on Linux/Mac or 'python' on Windows
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# --- Asset Pipeline (Python - build_assets.py v2.3) ---
set(ASSET_SCRIPT "${CMAKE_SOURCE_DIR}/tools/build_assets.py")
set(RAW_TEXTURES_DIR "${CMAKE_SOURCE_DIR}/assets/raw_textures")
set(GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

# Collect all PNGs so CMake knows when to re-run
file(GLOB_RECURSE RAW_PNGS CONFIGURE_DEPENDS "${RAW_TEXTURES_DIR}/*.png")

add_custom_command(
    OUTPUT "${GEN_DIR}/atlas_data.h" "${GEN_DIR}/atlas_data.c" "${GEN_DIR}/atlas.png"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${GEN_DIR}"
    COMMAND ${Python3_EXECUTABLE} "${ASSET_SCRIPT}" --input "${RAW_TEXTURES_DIR}" --output "${GEN_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GEN_DIR}/atlas.png" "${CMAKE_BINARY_DIR}/atlas.png"
    DEPENDS "${ASSET_SCRIPT}" ${RAW_PNGS}
    COMMENT "[ASSET PIPELINE] Packing texture atlas..."
    VERBATIM
)

add_custom_target(GenerateAtlas DEPENDS "${GEN_DIR}/atlas_data.h" "${GEN_DIR}/atlas_data.c" "${GEN_DIR}/atlas.png")

# --- Fetch Raylib ---
include(FetchContent)
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
)
FetchContent_MakeAvailable(raylib)

# --- Sources ---
set(SOURCES
    src/main.c

    # Generated Atlas
    "${GEN_DIR}/atlas_data.c"

    # Core Engine
    src/engine/core/engine.c
    src/engine/core/ui_helper.c
    src/engine/core/logger.c
    src/engine/core/scene_manager.c
    src/engine/core/input.c
    src/engine/core/entity_manager.c
    src/engine/core/asset_manager.c
    src/engine/core/cre_renderer.c
    src/engine/core/animation.c
    src/engine/core/cre_camera.c
    src/engine/core/cre_camera_utils.c
    src/engine/core/viewport.c
    src/engine/physics/physics_system.c
    src/engine/physics/spatial_hash.c
    
    # Game
    src/game/game.c
    src/game/entities/snake.c
    src/game/entities/food.c
    src/game/game_scenes.c
    src/game/game_systems.c
    src/game/menu.c
    src/game/game_over.c
    src/game/game_animation.c
)

# --- Executable ---
add_executable(${PROJECT_NAME} ${SOURCES})

# --- Dependencies ---
add_dependencies(${PROJECT_NAME} GenerateAtlas)

# --- Include Directories ---
target_include_directories(${PROJECT_NAME} PRIVATE src include "${GEN_DIR}")

# --- Linking ---
target_link_libraries(${PROJECT_NAME} raylib m)

# --- Assets ---
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR} PATTERN "raw_textures" EXCLUDE) 
file(COPY ${CMAKE_SOURCE_DIR}/src/game/config/settings.cfg DESTINATION ${CMAKE_BINARY_DIR}/assets/config)
