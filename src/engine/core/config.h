#ifndef CONFIG_H
#define CONFIG_H

// Change these values to change resolution
#define SCREEN_WIDTH  1920
#define SCREEN_HEIGHT 1080
#define CELL_SIZE     40

// These calculate automatically based on the values above
#define GRID_WIDTH    (SCREEN_WIDTH / CELL_SIZE)
#define GRID_HEIGHT   (SCREEN_HEIGHT / CELL_SIZE)

#endif // CONFIG_H