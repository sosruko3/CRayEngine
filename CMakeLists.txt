cmake_minimum_required(VERSION 3.14)
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "In-source builds are not allowed. Please run 'cmake -B build' and 'cmake --build build'.")
endif()

project(CRayEngine VERSION 0.9 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

# Use local raylib for now, will be removing it in a few weeks anyway.
set(BUILD_EXAMPLES    OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES       OFF CACHE BOOL "" FORCE)
set(BUILD_RESOURCES   OFF CACHE BOOL "" FORCE)
set(USE_EXTERNAL_GLFW OFF CACHE BOOL "" FORCE)

add_subdirectory(src/external/raylib)

# --- Sources ---
set(SOURCES
    src/main.cpp

    # Generated Atlas
    src/assets/atlas/atlas_data.c

    # Core Engine
    src/engine/core/cre_engine.cpp
    src/engine/core/cre_logger.cpp
    src/engine/core/cre_commandBus.cpp
    src/engine/scene/cre_sceneManager.cpp
    src/engine/platform/cre_input.cpp
    src/engine/platform/cre_viewport.cpp
    src/engine/ecs/cre_entityManager.cpp
    src/engine/ecs/cre_entityAPI.cpp
    src/engine/ecs/cre_entitySystem.cpp
    src/engine/loaders/cre_assetManager.cpp
    src/engine/systems/audio/cre_audioSystem.cpp
    src/engine/systems/audio/cre_audioAPI.cpp
    src/engine/systems/render/cre_rendererCore.cpp
    src/engine/systems/render/cre_renderSystem.cpp
    src/engine/systems/render/cre_renderAPI.cpp
    src/engine/systems/animation/cre_animationSystem.cpp
    src/engine/systems/animation/cre_animationAPI.cpp
    src/engine/systems/camera/cre_cameraSystem.cpp
    src/engine/systems/camera/cre_cameraUtils.cpp
    src/engine/systems/camera/cre_cameraAPI.cpp
    src/engine/systems/physics/cre_physicsSystem.cpp
    src/engine/systems/physics/cre_spatialHash.cpp
    src/engine/systems/physics/cre_physicsAPI.cpp
    src/engine/systems/debug/cre_debugSystem.cpp
    src/engine/systems/debug/cre_profilerSystem.cpp
    # Game
    src/game/game.cpp
    src/game/game_scenes.cpp
    src/game/controlSystem.cpp
    src/game/game_prototypes.cpp
    src/game/menu.cpp
    src/game/game_over.cpp
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

# --- Libraries included  ---
add_library(miniaudio_lib STATIC src/external/miniaudio/miniaudio_impl.c)

# --- Linking miniaudio ---
target_link_libraries(miniaudio_lib PRIVATE m pthread dl)

# --- Include Directories ---
target_include_directories(${PROJECT_NAME} PRIVATE src "${GEN_DIR}")

# --- Linking ---
target_link_libraries(${PROJECT_NAME} raylib miniaudio_lib)



# --- Assets ---
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR} PATTERN "raw_textures" EXCLUDE)
file(COPY ${CMAKE_SOURCE_DIR}/src/game/config/settings.cfg DESTINATION ${CMAKE_BINARY_DIR}/assets/config)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "Build Type is Debug: Enabling Strict C++20 Warnings & Sanitizers")
  
  if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX:vla /permissive-)
  else()
    target_compile_options(${PROJECT_NAME} PRIVATE 
        -Wall -Wextra -Wpedantic -Wshadow 
        -Werror=vla -Wdouble-promotion -Wnull-dereference 
        -Wimplicit-fallthrough -Wundef -Wformat=2
        -fsanitize=address,undefined 
        -fno-omit-frame-pointer 
        -Wshadow -Wswitch-enum
        -Wundef -Wformat=2
        -Wconversion -Wsign-conversion  
        # step3= -Wold-style-cast -Wcast-qual    
        # step4, might need to remove "-Werror=vla" for this = -Werror
    )
    
    target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address,undefined)
  endif()
endif()

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
