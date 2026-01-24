#ifndef CONFIG_H
#define CONFIG_H

#define GAME_VIRTUAL_HEIGHT 1080.0f // Change these values to change resolution
#define TARGET_FRAMERATE 120 // FOR DEBUG.Changed this but also will add 120hz support completely (add accumulator)
#define MIN_ZOOM 0.2f // Change these zooms to 0.4 and 4.0f later on, current numbers are for DEBUG!
#define MAX_ZOOM 5.0f
#define CAMERA_CULL_MARGIN 256.0f
#define SPATIAL_GRID_SIZE 128 // 128 pixel is a standart bucket size
#define SPATIAL_GRID_SHIFT 7 // 2^7 = 128
#define SPATIAL_HASH_SIZE 4096 // Size of the hash table (number of buckets)
// Total memory pool for nodes.Number is big in case entites touch multiple of them. In theory this amount can withstand 10k entity.
#define SPATIAL_MAX_NODES 40000

#define atlasDir      "src/game/atlas/atlas.png"
#define MAX_ENTITIES 10000
#define MAX_VISIBLE_ENTITIES 5000

// Will remove these soon.
#define GRID_WIDTH    (SCREEN_WIDTH / CELL_SIZE)
#define GRID_HEIGHT   (SCREEN_HEIGHT / CELL_SIZE)
#define SCREEN_WIDTH  1920
#define SCREEN_HEIGHT 1080
#define CELL_SIZE     40


#endif