#ifndef FOOD_H
#define FOOD_H

#include "raylib.h"

typedef struct Food {
    Vector2 position;
    Color color;
    bool active;
} Food;
void InitFood(void);
void UpdateFood(void);
void DrawFood(void);
Vector2 GetFoodPosition(void);
void SpawnFood(int gridWith, int gridHeight);
#endif // FOOD_H