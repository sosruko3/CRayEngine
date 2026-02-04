#ifndef CONFIG_H
#define CONFIG_H

#define GAME_VIRTUAL_HEIGHT 1080.0f // Change these values to change resolution
#define TARGET_FRAMERATE 60 // FOR DEBUG.
#define MIN_ZOOM 0.2f
#define MAX_ZOOM 5.0f
#define CAMERA_CULL_MARGIN 256.0f

#define SPATIAL_GRID_SIZE 128  // 128 pixel is a standart bucket size
#define SPATIAL_GRID_SHIFT 7   // 2^7 = 128. For bit shifting.
#define SPATIAL_HASH_SIZE 16384 // Size of the hash table (number of buckets)
// Total memory pool for nodes.Number should be ~4x of MAX_ENTITIES for safety.
#define SPATIAL_MAX_NODES 80000

// ============================================================================
// Physics Configuration
// ============================================================================
#define PHYS_GRAVITY_DEF_X 0.0f       // Default gravity X (typically 0 for side-scrollers)
#define PHYS_GRAVITY_DEF_Y 0.0f       // Default gravity Y (positive = down in screen coords)
#define PHYS_SUB_STEPS 4              // Number of physics sub-steps per frame
#define PHYS_SOLVER_ITERATIONS 8      // Collision solver iterations per sub-step
#define PHYS_SLEEP_EPSILON 2.0f       // Velocity threshold for sleep (squared internally)
#define PHYS_SLOP 0.01f               // Penetration allowance before correction
#define PHYS_CORRECTION_PERCENT 0.5f  // Position correction strength (0.0-1.0)
#define PHYS_MAX_NEIGHBOURS 128       // Max collision candidates per entity

#define atlasDir      "atlas.png"
#define MAX_ENTITIES 16384
#define MAX_VISIBLE_ENTITIES 4096

// Will remove these soon.
#define GRID_WIDTH    (SCREEN_WIDTH / CELL_SIZE)
#define GRID_HEIGHT   (SCREEN_HEIGHT / CELL_SIZE)
#define SCREEN_WIDTH  1920
#define SCREEN_HEIGHT 1080
#define CELL_SIZE     40


#endif