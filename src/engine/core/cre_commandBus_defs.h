#ifndef CRE_COMMANDBUS_DEFS_H
#define CRE_COMMANDBUS_DEFS_H

#include <stdint.h>

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
    CMD_ENTITY_DESTROY,
    CMD_ENTITY_ADD_COMPONENT,
    CMD_ENTITY_SET_PIVOT,

    // Animation commands
    CMD_ANIM_PLAY = CMD_DOMAIN_ANIM,
    CMD_ANIM_STOP,
    CMD_ANIM_PAUSE,
    CMD_ANIM_RESUME,

    // Render commands
    CMD_RENDER_SETDEPTHMATH = CMD_DOMAIN_RENDER,
    
    CMD_TYPE_COUNT // Render domain would count this as well, fix this.
} CommandType;

// ============================================================================
// Command Payload Structures (4-byte aligned)
// ============================================================================

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
    uint16_t type; // Entity type
    float x;
    float y;
} CommandPayloadSpawn;

typedef struct {
    float wX;
    float wY;
    float wH;
    uint8_t shiftBatch;
    uint8_t shiftDepth;
    uint8_t _pad[2];
} CommandPayloadRenderDepth;

#endif