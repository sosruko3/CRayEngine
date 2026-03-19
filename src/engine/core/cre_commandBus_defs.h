#ifndef CRE_COMMANDBUS_DEFS_H
#define CRE_COMMANDBUS_DEFS_H

#include "engine/core/cre_types.h"
#include <stdbool.h>
#include <stdint.h>

// Command Flags
constexpr uint8_t CMD_PHYS_FLAG_STATIC = (1 << 0); // Bit 0: Wall/Static

// Animation Command Flags
constexpr uint8_t ANIM_FLAG_FORCE_RESET = (1 << 0); // Bit 0: 1 = Force restart
constexpr uint8_t ANIM_FLAG_LOOP_OVERRIDE =
    (1 << 1); // Bit 1: 1 = Force loop (example)

constexpr uint32_t CMD_DOMAIN_MASK = 0xFF00U;
constexpr uint32_t CMD_DOMAIN_PHYS = 0x0100U;
constexpr uint32_t CMD_DOMAIN_ENTITY = 0x0200U;
constexpr uint32_t CMD_DOMAIN_ANIM = 0x0300U;
constexpr uint32_t CMD_DOMAIN_RENDER = 0x0400U;
constexpr uint32_t CMD_DOMAIN_CAMERA = 0x0500U;
constexpr uint32_t CMD_DOMAIN_AUDIO = 0x0600U;

// ============================================================================
// Command Types
// ============================================================================
// Use enum classes here.
typedef enum CommandType {
  CMD_NONE = 0,
  // Physics commands
  CMD_PHYS_TELEPORT = CMD_DOMAIN_PHYS,
  CMD_PHYS_SET_VELOCITY,
  CMD_PHYS_LOAD_STATIC,
  CMD_PHYS_DEFINE,
  CMD_PHYS_RESET,
  CMD_PHYS_APPLY_IMPULSE,
  CMD_PHYS_SET_DRAG,
  CMD_PHYS_SET_GRAVITY_SCALE,
  CMD_PHYS_SET_MATERIAL,
  CMD_PHYS_SET_GRAVITY,

  // Entity commands
  CMD_ENTITY_SPAWN = CMD_DOMAIN_ENTITY,
  CMD_ENTITY_SPAWN_UNTRACKED,
  CMD_ENTITY_CLONE,
  CMD_ENTITY_DESTROY,
  CMD_ENTITY_ADD_COMPONENT,
  CMD_ENTITY_REMOVE_COMPONENT,
  CMD_ENTITY_SET_PIVOT,
  CMD_ENTITY_SET_TYPE,
  CMD_ENTITY_ADD_FLAGS,
  CMD_ENTITY_REMOVE_FLAGS,
  CMD_ENTITY_RESET,

  // Animation commands
  CMD_ANIM_PLAY = CMD_DOMAIN_ANIM,
  CMD_ANIM_STOP,
  CMD_ANIM_PAUSE,
  CMD_ANIM_RESUME,
  CMD_ANIM_SET_SPEED,
  CMD_ANIM_SET_FRAME,
  CMD_ANIM_SET_LOOP,

  // Render commands
  CMD_RENDER_SETDEPTHMATH = CMD_DOMAIN_RENDER,
  CMD_RENDER_SET_SPRITE,
  CMD_RENDER_SET_COLOR,
  CMD_RENDER_SET_VISIBLE,
  CMD_RENDER_SET_VISUAL_SCALE,
  CMD_RENDER_SET_ROTATION,
  CMD_RENDER_SET_LAYER,

  // Camera commands
  CMD_CAM_SET_ACTIVE = CMD_DOMAIN_CAMERA,
  CMD_CAM_SET_PRIORITY,
  CMD_CAM_SET_ZOOM,
  CMD_CAM_SET_ROTATION,
  CMD_CAM_SET_FOLLOW,
  CMD_CAM_DISABLE_FOLLOW,

  // Audio commands
  CMD_AUDIO_GROUP_INIT = CMD_DOMAIN_AUDIO,
  CMD_AUDIO_SET_MASTER_VOLUME,
  CMD_AUDIO_SET_LISTENER_POSITION,
  CMD_AUDIO_GROUP_SET_VOLUME,
  CMD_AUDIO_GROUP_SET_PITCH,
  CMD_AUDIO_GROUP_SET_PAN,
  CMD_AUDIO_PLAY_ONESHOT,
  CMD_AUDIO_SOUND_LOAD,
  CMD_AUDIO_SOUND_UNLOAD,
  CMD_AUDIO_SOUND_PLAY,
  CMD_AUDIO_SOUND_PAUSE,
  CMD_AUDIO_SOUND_STOP,
  CMD_AUDIO_SOUND_SET_VOLUME,
  CMD_AUDIO_SOUND_SET_PITCH,
  CMD_AUDIO_SOUND_SET_PAN,
  CMD_AUDIO_SOUND_SET_LOOPING,
  CMD_AUDIO_SOUND_SET_SPATIALIZATION,
  CMD_AUDIO_SOUND_SET_POSITION,
  CMD_AUDIO_SOUND_SET_ATTENUATION
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
  uint8_t value;
} CommandPayloadU8;

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
  uint8_t flags;       // e.g., static,
  // renomed 2 bytes of _padding
  float drag;
} CommandPayloadPhysDef;

typedef struct {
  Entity prototype;
  creVec2 position;
} CommandPayloadEntityClone;

typedef struct {
  Entity targetEntity;
  float smoothSpeed;
  creVec2 offset;
} CommandPayloadCamFollow;

typedef struct {
  creColor color;
} CommandPayloadColor;

typedef struct {
  float wX;
  float wY;
  float wH;
  uint8_t shiftBatch;
  uint8_t shiftDepth;
  // removed 2 bytes of _padding
} CommandPayloadRenderDepth;

typedef struct {
  AudioID id;
} CommandPayloadAudioID;

typedef struct {
  AudioID id;
  AudioSourceID sourceID;
  AudioGroupID groupID;
  AudioUsageType usageType;
} CommandPayloadAudioLoad;

typedef struct {
  AudioGroupID groupID;
  // removed 3 bytes of _padding
  float value;
} CommandPayloadAudioGroup;

typedef struct {
  AudioID id;
  float value;
} CommandPayloadAudioF32;

typedef struct {
  AudioID id;
  bool value;
  // removed 3 byte of _padding
} CommandPayloadAudioB8;

typedef struct {
  AudioID id;
  creVec2 value;
} CommandPayloadAudioVec2;

typedef struct {
  AudioSourceID sourceid;
  AudioGroupID groupid;
  // removed 1 byte of _padding
} CommandPayloadAudioOneShot;

#endif
