#ifndef CRE_TYPES_H
#define CRE_TYPES_H
#include <stdint.h>

typedef struct Entity {
    uint32_t id;         ///< Index into the registry arrays
    uint32_t generation; ///< Generation counter for validation
} Entity;

/** Invalid entity sentinel value */
#define ENTITY_INVALID ((Entity){ .id = UINT32_MAX, .generation = 0 })

/** Check if an entity handle is valid (not the sentinel) */
#define ENTITY_IS_VALID(e) ((e).id != UINT32_MAX)

typedef struct creVec2 {
    float x;
    float y;
} creVec2;

typedef struct creColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} creColor;

typedef struct creRectangle {
    float x;                // Rectangle top-left corner position x
    float y;                // Rectangle top-left corner position y
    float width;            // Rectangle width
    float height;           // Rectangle height
} creRectangle;

#endif