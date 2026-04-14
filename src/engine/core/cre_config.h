#ifndef CRE_CONFIG_H
#define CRE_CONFIG_H

#include <stdint.h>

constexpr float GAME_VIRTUAL_HEIGHT = 1080.0f;
constexpr int TARGET_FRAMERATE = 60;
constexpr float MIN_ZOOM = 0.2f;
constexpr float MAX_ZOOM = 4.0f;
constexpr float CAMERA_CULL_MARGIN = 256.0f;

#define SPATIAL_GRID_SIZE 128   // 128 pixel is a standart bucket size
#define SPATIAL_GRID_SHIFT 7    // 2^7 = 128. For bit shifting.
#define SPATIAL_HASH_SIZE 16384 // Size of the hash table (number of buckets)
// Total memory pool for nodes.Number should be ~4x of MAX_ENTITIES for safety.
#define SPATIAL_MAX_NODES 80000

// ============================================================================
// Physics Configuration
// ============================================================================
constexpr float PHYS_GRAVITY_DEF_X =
    0.0f; // Default gravity X (typically 0 for side-scrollers)
constexpr float PHYS_GRAVITY_DEF_Y =
    0.0f; // Default gravity Y (positive = down in screen coords)
constexpr int PHYS_SUB_STEPS = 4; // Number of physics sub-steps per frame
constexpr int PHYS_SOLVER_ITERATIONS =
    16; // Collision solver iterations per sub-step
constexpr float PHYS_SLEEP_EPSILON =
    2.0f; // Velocity threshold for sleep (squared internally)
constexpr float PHYS_SLOP = 0.1f; // Penetration allowance before correction
constexpr float PHYS_CORRECTION_PERCENT =
    0.5f; // Position correction strength (0.0-1.0)
constexpr int PHYS_MAX_NEIGHBOURS = 128; // Max collision candidates per entity

#define atlasDir "atlas.png"
#define MAX_ENTITIES 16384
#define MAX_VISIBLE_ENTITIES 8192
constexpr uint32_t MAX_CAMERAS = 8;

// Will remove these soon.
constexpr float SCREEN_WIDTH = 1920.0f;
constexpr float SCREEN_HEIGHT = 1080.0f;

#endif
