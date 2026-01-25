cmake_minimum_required(VERSION 3.14)
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are not allowed. Please run 'cmake -B build' and 'cmake --build build'.")
endif()

project(CRayEngine VERSION 0.6)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 1. Check if Node/NPM is installed
find_program(NODE_EXECUTABLE node)
find_program(NPM_EXECUTABLE npm)
if(NOT NODE_EXECUTABLE OR NOT NPM_EXECUTABLE)
    message(FATAL_ERROR "Node.js and npm are required for the asset pipeline. Please install them.")
endif()

# 2. Install free-tex-packer dependencies
set(TOOLS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tools")
set(PACKER_SCRIPT "${TOOLS_DIR}/pack_atlas.js")

if(NOT EXISTS "${TOOLS_DIR}/node_modules")
    message(STATUS "Installing Asset Packer tools...")
    execute_process(
        COMMAND ${NPM_EXECUTABLE} install
        WORKING_DIRECTORY ${TOOLS_DIR}
        RESULT_VARIABLE npm_result
    )
    if(NOT npm_result EQUAL 0)
        message(FATAL_ERROR "Failed to install npm packages.")
    endif()
endif()

# 3. Define the Packer Command
set(RAW_ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets/raw_textures")
set(GENERATED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/game/atlas")
set(TEMPLATE_FILE "${TOOLS_DIR}/header_template.mustache")

# Collect all PNGs so CMake knows when to re-run
file(GLOB_RECURSE RAW_PNGS CONFIGURE_DEPENDS "${RAW_ASSETS_DIR}/*.png")

add_custom_command(
    COMMAND ${CMAKE_COMMAND} -E make_directory "${GENERATED_DIR}"
    OUTPUT "${GENERATED_DIR}/atlas.png" "${GENERATED_DIR}/atlas_data.h"

    COMMAND ${NODE_EXECUTABLE} "${PACKER_SCRIPT}" 
            "${RAW_ASSETS_DIR}" 
            "${GENERATED_DIR}" 
            "${TEMPLATE_FILE}"

    COMMAND ${CMAKE_COMMAND} -E rename "${GENERATED_DIR}/atlas.h" "${GENERATED_DIR}/atlas_data.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GENERATED_DIR}/atlas.png" "${CMAKE_BINARY_DIR}/atlas.png"
    DEPENDS ${RAW_PNGS} "${TEMPLATE_FILE}" "${PACKER_SCRIPT}" "${RAW_ASSETS_DIR}"
    COMMENT "Packing textures into atlas..."
    VERBATIM
)

# 4. Create a Target so your game depends on it
add_custom_target(GenerateAtlas DEPENDS "${GENERATED_DIR}/atlas_data.h")

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
target_include_directories(${PROJECT_NAME} PRIVATE src include "${GENERATED_DIR}")

# --- Linking ---
target_link_libraries(${PROJECT_NAME} raylib m)

# --- Assets ---
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR} PATTERN "raw_textures" EXCLUDE) 
file(COPY ${CMAKE_SOURCE_DIR}/src/game/config/settings.cfg DESTINATION ${CMAKE_BINARY_DIR}/assets/config)
