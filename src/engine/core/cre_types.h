#ifndef CRE_TYPES_H
#define CRE_TYPES_H
#include <stdint.h>
#include <stddef.h>

struct EntityRegistry;
struct CommandBus;

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

struct Arena {
    uint8_t* base_ptr = nullptr; 
    size_t capacity = 0;
    size_t offset = 0;
};

struct EngineContext {
  Arena masterArena;
  Arena entityArena;
  Arena physicsArena;
  Arena audioArena;
  Arena busArena;
  Arena frameArena; 
  EntityRegistry* reg; 
  CommandBus* bus;
};

// Enum Forward Declaration.
enum AudioSourceID : uint16_t;
enum AudioGroupID : uint8_t;
enum AudioUsageType : uint8_t;

struct Entity {
  uint32_t id;         ///< Index into the registry arrays
  uint32_t generation; ///< Generation counter for validation
};

#define ENTITY_ID_MAX UINT32_MAX
#define ENTITY_INVALID (Entity{.id = ENTITY_ID_MAX, .generation = 0})
#define ENTITY_IS_VALID(e) ((e).id != ENTITY_ID_MAX)
#define ENTITY_MATCH(e1, e2)                                                   \
  ((e1).id == (e2).id && (e1).generation == (e2).generation)

struct AudioID {
  uint16_t index;
  uint16_t gen;
};

struct creVec2 {
  float x;
  float y;
};

struct creColor {
  uint8_t r; // red
  uint8_t g; // green
  uint8_t b; // blue
  uint8_t a; // alpha
};

struct creRectangle {
  float x;      // Rectangle top-left corner position x
  float y;      // Rectangle top-left corner position y
  float width;  // Rectangle width
  float height; // Rectangle height
};

#endif
