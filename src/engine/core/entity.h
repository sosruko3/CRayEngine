#ifndef ENTITY_H
#define ENTITY_H

#include "raylib.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t id;
    uint32_t generation;
} Entity;

typedef struct {
    bool isActive;
    int type;
    Vector2 scale;
    Vector2 position;
    Vector2 velocity;
    Vector2 size;
    float rotation;
    float radius;
    Color color;
    uint32_t generation;
    uint8_t _optCache[8];
} EntityData;
// _optCache is to make the struct 64byte, it is not used.

#endif
