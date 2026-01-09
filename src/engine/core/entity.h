#ifndef ENTITY_H
#define ENTITY_H

#include "raylib.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t id;
} Entity;

typedef struct {
    bool isActive;
    int type;
    Vector2 scale;
    Vector2 position;
    Vector2 velocity;
    float rotation;
    float radius;
    Color color;
} EntityData;

#endif
