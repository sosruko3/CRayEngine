/**
 * @file cre_entityRegistry.h
 * @brief Packed Parallel Structure of Arrays (SoA) Entity System
 *
 * Data-Oriented Design entity registry with:
 * - 64-byte aligned arrays for cache efficiency
 * - Generational indices for safe entity handles
 * - FreeList for O(1) entity recycling
 * - Separated data highways for SIMD-friendly iteration
 */

#ifndef CRE_ENTITYREGISTRY_H
#define CRE_ENTITYREGISTRY_H

#include "cre_components.h"
#include "cre_entityEvents.h"
#include "engine/core/cre_config.h"
#include "engine/core/cre_types.h"
#include <stdalign.h>
#include <stdbool.h>
#include <stdint.h>

// These three will be removed soon, just temporary for systems to work.
#define RENDER_BATCH_DEFAULT 0u
#define RENDER_BATCH_PLAYER 1u
#define RENDER_BATCH_ENEMY 2u

// ============================================================================
// Component Mask Bits (uint64_t component_masks[])
// ============================================================================
// These bits indicate which components an entity possesses.

constexpr uint64_t COMP_NONE = (0ULL);
constexpr uint64_t COMP_SPRITE = (1ULL << 1);
constexpr uint64_t COMP_ANIMATION = (1ULL << 2);
constexpr uint64_t COMP_PHYSICS = (1ULL << 3);
constexpr uint64_t COMP_COLLISION_Circle = (1ULL << 4);
constexpr uint64_t COMP_COLLISION_AABB = (1ULL << 5);
constexpr uint64_t COMP_SOUND = (1ULL << 6);
constexpr uint64_t COMP_AI = (1ULL << 7);
constexpr uint64_t COMP_SHADER = (1ULL << 8);
constexpr uint64_t COMP_CAMERA = (1ULL << 9);

// Reserve bits 6-31 for future component types
// Bits 32-63 available for game-specific components

// ============================================================================
// State Flags (uint64_t state_flags[])
// ============================================================================
// Bits 0-15:  Behavioral toggles (engine state)
// Bits 16-31: Collision Layer (which layer this entity belongs to)
// Bits 32-47: Collision Mask (which layers this entity collides with)
// Bits 48-63: Reserved for future use

// --- Behavioral Flags (Bits 0-15) ---
constexpr uint64_t FLAG_ACTIVE = (1ULL << 0);  ///< Is this slot in use?
constexpr uint64_t FLAG_VISIBLE = (1ULL << 1); ///< Should renderer draw it?
constexpr uint64_t FLAG_ALWAYS_AWAKE =
    (1ULL << 2); ///< Never enters sleep state
constexpr uint64_t FLAG_SLEEPING =
    (1ULL << 3); ///< Currently sleeping (skip physics)
constexpr uint64_t FLAG_CULLED =
    (1ULL << 4); ///< Outside camera view, skip rendering
constexpr uint64_t FLAG_PERSISTENT =
    (1ULL << 5);                              ///< Survives scene transitions
constexpr uint64_t FLAG_STATIC = (1ULL << 6); ///< Static in physics.
constexpr uint64_t FLAG_ANIM_PAUSED =
    (1ULL << 7); ///< Entity's animation is paused.
constexpr uint64_t CLONE_FLAGS_SCRUB_MASK =
    (FLAG_ACTIVE | FLAG_CULLED | FLAG_SLEEPING);
// Bits 8-15 reserved for future engine flags

// --- Collision Layer/Mask (64-bit version) ---
#define LAYER_SHIFT 16ULL
#define MASK_SHIFT 32ULL
#define LAYER_MASK_VAL 0xFFFFULL
#define LAYER_BITS (LAYER_MASK_VAL << LAYER_SHIFT)
#define MASK_BITS (LAYER_MASK_VAL << MASK_SHIFT)

// Macros for collision layer/mask manipulation
#define SET_LAYER(l) (static_cast<uint64_t>(l) << LAYER_SHIFT)
#define GET_LAYER(flags) (((flags) >> LAYER_SHIFT) & 0xFFFFULL)
#define SET_MASK(m) (static_cast<uint64_t>(m) << MASK_SHIFT)
#define GET_MASK(flags) (((flags) >> MASK_SHIFT) & 0xFFFFULL)

// Clear and set helpers
#define CLEAR_LAYER(flags)                                                     \
  static_cast<uint32_t>(flags) & ~(0xFFFFULL << LAYER_SHIFT)
#define CLEAR_MASK(flags)                                                      \
  (static_cast<uint32_t>(flags) & ~(0xFFFFULL << MASK_SHIFT)

// ============================================================================
// Entity Registry (Packed Parallel SoA)
// ============================================================================

/**
 * @brief The core data structure holding all entity data in SoA layout.
 *
 * All arrays are 64-byte aligned for optimal cache line usage and SIMD.
 * Data is stored in "parallel" arrays - same index across all arrays
 * refers to the same entity.
 */
struct EntityRegistry {
  alignas(64) creVec2 pos[MAX_ENTITIES]; ///< Position
  alignas(64) creVec2 vel[MAX_ENTITIES]; ///< Velocity
  alignas(64) uint64_t
      component_masks[MAX_ENTITIES]; ///< Component presence bits
  alignas(64) uint64_t
      state_flags[MAX_ENTITIES]; ///< Behavioral flags + collision data
  alignas(64) uint8_t render_layer[MAX_ENTITIES]; ///< Render_layer
  alignas(64) uint8_t batch_ids[MAX_ENTITIES];    ///< Render batch lookup index

  alignas(64) creVec2 size[MAX_ENTITIES];        ///< Size w/h
  alignas(64) uint8_t material_id[MAX_ENTITIES]; ///< Material ID for physics.
  alignas(64) float drag[MAX_ENTITIES]; ///< Air resistance etc. for physics.
  alignas(64) float inv_mass[MAX_ENTITIES];      ///< Physics Mass
  alignas(64) float gravity_scale[MAX_ENTITIES]; ///< Gravity Scales
  alignas(64) float rotation[MAX_ENTITIES];      ///< Rotation in degrees

  // change sprite_ids to uint32_t?? Not sure about this one.
  alignas(64) uint16_t sprite_ids[MAX_ENTITIES];  ///< Sprite/texture ID
  alignas(64) creColor colors[MAX_ENTITIES];      ///< Tint color
  alignas(64) creVec2 pivot[MAX_ENTITIES];        ///< Pivot of sprite
  alignas(64) creVec2 visual_scale[MAX_ENTITIES]; ///< Visual Scale
  // Animation SoA arrays - Dynamic State (managed by animationSystem)
  alignas(64) float anim_timers[MAX_ENTITIES]; ///< Time accumulator for frame
                                               ///< advance
  alignas(
      64) float anim_speeds[MAX_ENTITIES]; ///< Speed multiplier (1.0f = normal)
  alignas(
      64) uint16_t anim_ids[MAX_ENTITIES]; ///< Current animation ID (AnimID)
  alignas(64) uint16_t anim_frames[MAX_ENTITIES]; ///< Current frame index
  alignas(64) bool anim_finished[MAX_ENTITIES];   ///< True if non-looping anim
                                                  ///< completed

  // Animation SoA arrays - Baked Constants
  alignas(
      64) float anim_base_durations[MAX_ENTITIES]; ///< Seconds per frame (from
                                                   ///< def->defaultSpeed)
  alignas(64) uint16_t
      anim_frame_counts[MAX_ENTITIES]; ///< Total frames (from def->frameCount)
  alignas(64) uint16_t
      anim_start_sprites[MAX_ENTITIES];      ///< First sprite ID (from
                                             ///< def->startSpriteID)
  alignas(64) bool anim_loops[MAX_ENTITIES]; ///< Loop flag (from def->loop)
  alignas(64) CameraComponent cameras[MAX_CAMERAS];
  alignas(64) uint32_t camera_count;

  alignas(64) uint16_t types[MAX_ENTITIES];       ///< Entity type ID
  alignas(64) uint32_t generations[MAX_ENTITIES]; ///< Generation counters
  alignas(64) uint32_t free_list[MAX_ENTITIES];   ///< Stack of free indices

  alignas(64) uint32_t free_count;   ///< Number of free slots
  alignas(64) uint32_t active_count; ///< Number of active entities
  alignas(64) uint32_t
      max_used_bound; ///< Highest index ever used (optimization hint)

  alignas(64) EntityEventDispatcher events; ///< Lifecycle hook dispatcher state
};

static_assert(alignof(EntityRegistry) == 64,
              "EntityRegistry MUST be 64-byte aligned for cache efficiency!");
static_assert(sizeof(EntityRegistry) % 64 == 0,
              "EntityRegistry size MUST be a multiple of 64 bytes!");

static inline bool EntityRegistry_IsAlive(const EntityRegistry &reg, Entity e) {
  if (e.id >= MAX_ENTITIES)
    return false;
  if (!(reg.state_flags[e.id] & FLAG_ACTIVE))
    return false;
  return reg.generations[e.id] == e.generation;
}

static inline bool EntityRegistry_IsValid(const EntityRegistry &reg, Entity e) {
  if (e.id >= MAX_ENTITIES)
    return false;
  return (reg.generations[e.id] == e.generation);
}

#endif
