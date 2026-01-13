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
// --- These control the engine state for the entity (Bits 0-15)
#define FLAG_ACTIVE        (1 << 0) // (Is this slot in use)
#define FLAG_VISIBLE       (1 << 1) // (Should Render draw it)
#define FLAG_SOLID         (1 << 2) // NOTE:Will change this later on,useless since we use layers and masks for collision.
#define FLAG_DAMAGED       (1 << 3) // Will change this to something else, do not use.
#define FLAG_ALWAYS_AWAKE  (1 << 4) // (No sleeping for physics)
#define FLAG_SLEEPING      (1 << 5) // (Physics are on sleep)
#define FLAG_BOUNCY        (1 << 6) // This entity transfers velocity/bounces
#define FLAG_ANIMATED      (1 << 7) // Is using Animations

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
    Color color; // can change this to "tint" , think later
    float rotation;
    float restitution; // For bounciness, planning to implement later
    uint32_t generation;
    uint32_t flags; // replaces "isActive" etc.
    // 2 byte
    uint16_t spriteID; // Replaced textureID(was not used) with spriteID.
    uint16_t type;
    // optCache
    uint8_t _optCache[16];
} EntityData;
// _optCache is to make the struct 64byte, it is not used.

#endif
