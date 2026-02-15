cmake_minimum_required(VERSION 3.14)
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are not allowed. Please run 'cmake -B build' and 'cmake --build build'.")
endif()

project(CRayEngine VERSION 0.8)

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
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/src/assets/atlas"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${GEN_DIR}" "${CMAKE_SOURCE_DIR}/src/assets/atlas"
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
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG master
    GIT_PROGRESS TRUE
)
set(BUILD_EXAMPLES    OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES       OFF CACHE BOOL "" FORCE)
set(BUILD_RESOURCES   OFF CACHE BOOL "" FORCE)
set(USE_EXTERNAL_GLFW OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

# --- Sources ---
set(SOURCES
    src/main.c

    # Generated Atlas
    src/assets/atlas/atlas_data.c

    # Core Engine
    src/engine/core/cre_engine.c
    src/engine/core/cre_uiHelper.c
    src/engine/core/cre_logger.c
    src/engine/core/cre_commandBus.c
    src/engine/scene/cre_sceneManager.c
    src/engine/platform/cre_input.c
    src/engine/platform/cre_viewport.c
    src/engine/ecs/cre_entityManager.c
    src/engine/ecs/cre_entityAPI.c
    src/engine/ecs/cre_entitySystem.c
    src/engine/loaders/cre_assetManager.c
    src/engine/systems/render/cre_RendererCore.c
    src/engine/systems/render/cre_RenderSystem.c
    src/engine/systems/render/cre_renderAPI.c
    src/engine/systems/animation/cre_animationSystem.c
    src/engine/systems/animation/cre_animationAPI.c
    src/engine/systems/camera/cre_cameraSystem.c
    src/engine/systems/camera/cre_cameraUtils.c
    src/engine/systems/camera/cre_cameraAPI.c
    src/engine/systems/physics/cre_physicsSystem.c
    src/engine/systems/physics/cre_spatialHash.c
    src/engine/systems/physics/cre_physicsAPI.c
    
    # Game
    src/game/game.c
    src/game/game_scenes.c
    src/game/controlSystem.c
    src/game/debugSystem.c
    src/game/menu.c
    src/game/game_over.c
)

# --- Executable ---
add_executable(${PROJECT_NAME} ${SOURCES})

# --- Link Time Optimization (LTO) ---
# This allows inlining functions across different .c files
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result OUTPUT output)
    
    if(result)
        message(STATUS "IPO/LTO is supported: Enabling")
        set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(WARNING "IPO/LTO is not supported: ${output}")
    endif()
endif()

# --- Dependencies ---
add_dependencies(${PROJECT_NAME} GenerateAtlas)

# --- Include Directories ---
target_include_directories(${PROJECT_NAME} PRIVATE src "${GEN_DIR}")

# --- Linking ---
target_link_libraries(${PROJECT_NAME} raylib m)



# --- Assets ---
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR} PATTERN "raw_textures" EXCLUDE) 
file(COPY ${CMAKE_SOURCE_DIR}/src/game/config/settings.cfg DESTINATION ${CMAKE_BINARY_DIR}/assets/config)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Build Type is Release: Enabling Aggressive Optimizations")
    
    if(MSVC)
        # Visual Studio Flags
        # /Ox = Full Optimization
        # /arch:AVX2 = Enable SIMD vectorization (Crucial for SoA)
        # /fp:fast = Faster floating point math (less precise, usually fine for games)
        target_compile_options(${PROJECT_NAME} PRIVATE /Ox /arch:AVX2 /fp:fast)
    else()
        # GCC / Clang Flags
        # -O3 = Maximum Optimization
        # -march=native = Use your specific CPU's instruction set (AVX/AVX2)
        # -ffast-math = trade IEEE precision for speed
        # -funroll-loops = Helps with the physics "for" loops
        target_compile_options(${PROJECT_NAME} PRIVATE -O3 -march=native -ffast-math -funroll-loops)
    endif()
endif()