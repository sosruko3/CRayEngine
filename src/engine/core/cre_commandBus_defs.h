#ifndef CRE_COMMANDBUS_DEFS_H
#define CRE_COMMANDBUS_DEFS_H

#include "engine/core/cre_types.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct Entity Entity;

// Command Flags
#define CMD_PHYS_FLAG_STATIC  (1 << 0) // Bit 0: Wall/Static
#define CMD_PHYS_FLAG_SENSOR  (1 << 1) // Bit 1: Trigger (Future proofing)
#define CMD_PHYS_FLAG_BULLET  (1 << 2) // Bit 2: CCD/Fast Mover (Future proofing)

// Animation Command Flags
#define ANIM_FLAG_FORCE_RESET  (1 << 0) // Bit 0: 1 = Force restart
#define ANIM_FLAG_LOOP_OVERRIDE (1 << 1) // Bit 1: 1 = Force loop (example)

#define CMD_DOMAIN_MASK    0xFF00
#define CMD_DOMAIN_PHYS    0x0100
#define CMD_DOMAIN_ENTITY  0x0200
#define CMD_DOMAIN_ANIM    0x0300
#define CMD_DOMAIN_RENDER  0x0400

// ============================================================================
// Command Types
// ============================================================================

typedef enum CommandType {
    CMD_NONE = 0,
    // Physics commands
    CMD_PHYS_MOVE = CMD_DOMAIN_PHYS,
    CMD_PHYS_SET_VELOCITY,
    CMD_PHYS_LOAD_STATIC,
    CMD_PHYS_DEFINE,
    CMD_PHYS_RESET,
    
    // Entity commands
    CMD_ENTITY_SPAWN = CMD_DOMAIN_ENTITY,
    CMD_ENTITY_SPAWN_UNTRACKED, 
    CMD_ENTITY_CLONE,
    CMD_ENTITY_DESTROY,
    CMD_ENTITY_ADD_COMPONENT,
    CMD_ENTITY_REMOVE_COMPONENT,
    CMD_ENTITY_SET_PIVOT,
    CMD_ENTITY_SET_TYPE,
    CMD_ENTITY_SET_FLAGS,
    CMD_ENTITY_RESET,
    CMD_ENTITY_CLEAR_FLAGS,

    // Animation commands
    CMD_ANIM_PLAY = CMD_DOMAIN_ANIM,
    CMD_ANIM_STOP,
    CMD_ANIM_PAUSE,
    CMD_ANIM_RESUME,
    CMD_ANIM_SETSPEED,
    CMD_ANIM_SETFRAME,
    CMD_ANIM_SETLOOP,

    // Render commands
    CMD_RENDER_SETDEPTHMATH = CMD_DOMAIN_RENDER,
    
    CMD_TYPE_COUNT // Render domain would count this as well, fix this.
} CommandType;

// ============================================================================
// Command Payload Structures (4-byte aligned)
// ============================================================================

// --- GENERIC / SHARED PAYLOADS ---
// Used for ANY command in the engine that just needs two floats 
typedef struct {
    creVec2 value;
} CommandPayloadVec2;

typedef struct {
    float value;
} CommandPayloadF32;

typedef struct {
    uint16_t value;
} CommandPayloadU16;

typedef struct {
    bool value;
} CommandPayloadB8;

typedef struct {
    uint64_t value;
} CommandPayloadU64;

// --- SPECIFIC PAYLOADS ---
// Used only when a command has a highly unique footprint

typedef struct {
    uint16_t animID;
    uint16_t flags;
} CommandPayloadAnim;

typedef struct {
    uint8_t material_id; // e.g., MAT_WOOF
    uint8_t flags;      // e.g., static
    uint8_t _padding[2];
    float drag;
} CommandPayloadPhysDef;

typedef struct { 
    // Not used yet. Will be implemented after sound system.
    uint16_t soundID;
    uint16_t _pad;
    float volume;
} CommandPayloadAudio;

typedef struct {
    Entity prototype;
    creVec2 position;
} CommandPayloadEntityClone;

typedef struct {
    float wX;
    float wY;
    float wH;
    uint8_t shiftBatch;
    uint8_t shiftDepth;
    uint8_t _pad[2];
} CommandPayloadRenderDepth;


#endif