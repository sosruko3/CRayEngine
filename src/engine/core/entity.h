#ifndef ENTITY_H
#define ENTITY_H

#include "raylib.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t id;
    uint32_t generation;
} Entity;

// bitmasks (flags)
// --- These control the engine state for the entity (Bits 0-7)
#define FLAG_ACTIVE       (1 << 0) // 1 = 0001( Is this slot in use)
#define FLAG_VISIBLE      (1 << 1) // 2 = 0010 ( Should Render draw it)
#define FLAG_SOLID        (1 << 2) // 4 == 0100 ( Should phyics check collisions)
#define FLAG_DAMAGED      (1 << 3) // 8 = 1000 // CHANGE THIS LATER ON, CAN BE CONSIDERED BAD CODE!!!!
#define FLAG_ALWAYS_AWAKE (1 << 4) // 16 = 10000 (No sleep for physics)


// Collision Tools (Generic)
// Engine provides the "math", not the "names"
#define LAYER_SHIFT 16
#define MASK_SHIFT  24

// QoL Macros for collision system
#define SET_LAYER(l)     ((l)<< LAYER_SHIFT)
#define SET_MASK(m)      ((m) << MASK_SHIFT)
#define GET_LAYER(flags) (((flags) >> LAYER_SHIFT) & 0xFF)
#define GET_MASK(flags)  (((flags) >> MASK_SHIFT)  & 0xFF)


typedef struct {
    // 8 byte
    Vector2 position;
    Vector2 velocity;
    Vector2 size;
    // 4 byte
    Color color;
    float rotation;
    float restitution; // For bounciness, planning to implement later
    uint32_t generation;
    uint32_t flags; // replaces "isActive"
    // 2 byte
    uint16_t textureID; // currently not used.
    uint16_t type;
    // optCache
    uint8_t _optCache[16];
} EntityData;
// _optCache is to make the struct 64byte, it is not used.
#endif
