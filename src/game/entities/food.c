#include "food.h"
#include "engine/core/config.h"
#include <stdlib.h>

static Food food = {0};

void InitFood(void)
{
    food.color = RED;
    food.active = true;
    // Initial spawn
    SpawnFood(GRID_WIDTH, GRID_HEIGHT);
}

void UpdateFood(void)
{
    // Food logic if needed (e.g animations)
}

void DrawFood(void)
{
    if (food.active)
    {
        DrawRectangle(
            (int)food.position.x * CELL_SIZE,
            (int)food.position.y * CELL_SIZE,
            CELL_SIZE,
            CELL_SIZE,
            food.color
        );
    }
}

Vector2 GetFoodPosition(void) 
{
    return food.position;
}

void SpawnFood(int gridWidth, int gridHeight)
{
    food.position = (Vector2)
    {
        (float)(GetRandomValue(0, gridWidth - 1)),
        (float)(GetRandomValue(0, gridHeight - 1))
    };
    food.active = true;
}