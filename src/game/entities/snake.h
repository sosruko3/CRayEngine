#ifndef SNAKE_H
#define SNAKE_H
#include "raylib.h"
#include <stdbool.h>

void InitSnake(int startX, int startY);
void UpdateSnake(void);
void DrawSnake(void);
Vector2 GetSnakeHeadPosition(void);
void GrowSnake(void);
bool CheckSnakeCollision(int gridWidth, int gridHeight);

#endif // SNAKE_H