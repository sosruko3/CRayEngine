#ifndef CRE_TYPES_H
#define CRE_TYPES_H
#include <stdint.h>

typedef struct Entity {
    uint64_t id         : 24 ;      ///< Index into the registry arrays
    uint64_t generation : 24 ;      ///< Generation counter for validation
    uint64_t unused     : 16 ;
} Entity;

// Max for 24bit.
#define ENTITY_ID_MAX 0xFFFFFF

/** Invalid entity sentinel value */
#define ENTITY_INVALID ((Entity){ .id = ENTITY_ID_MAX, .generation = 0 })

/** Check if an entity handle is valid (not the sentinel) */
#define ENTITY_IS_VALID(e) ((e).id != ENTITY_ID_MAX)

#define ENTITY_MATCH(e1, e2) ((e1).id == (e2).id && (e1).generation == (e2).generation)

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