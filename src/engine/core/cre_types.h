#ifndef CRE_TYPES_H
#define CRE_TYPES_H
#include <stdint.h>

// restrict macro
#if defined(__cplusplus)
#if defined(__clang__) || defined(__GNUC__)
#define restrict __restrict__
#elif defined(_MSC_VER)
#define restrict __restrict
#else
#define restrict
#endif
#endif

// Enum Forward Declaration.
enum AudioSourceID : uint16_t;
enum AudioGroupID : uint8_t;
enum AudioUsageType : uint8_t;

typedef struct Entity {
  uint32_t id;         ///< Index into the registry arrays
  uint32_t generation; ///< Generation counter for validation
} Entity;

#define ENTITY_ID_MAX UINT32_MAX
#define ENTITY_INVALID (Entity{.id = ENTITY_ID_MAX, .generation = 0})
#define ENTITY_IS_VALID(e) ((e).id != ENTITY_ID_MAX)
#define ENTITY_MATCH(e1, e2)                                                   \
  ((e1).id == (e2).id && (e1).generation == (e2).generation)

typedef struct {
  uint16_t index;
  uint16_t gen;
} AudioID;

typedef struct creVec2 {
  float x;
  float y;
} creVec2;

typedef struct creColor {
  uint8_t r; // red
  uint8_t g; // green
  uint8_t b; // blue
  uint8_t a; // alpha
} creColor;

typedef struct creRectangle {
  float x;      // Rectangle top-left corner position x
  float y;      // Rectangle top-left corner position y
  float width;  // Rectangle width
  float height; // Rectangle height
} creRectangle;

#endif
