/**
 * @file entity_registry.h
 * @brief Packed Parallel Structure of Arrays (SoA) Entity System
 * 
 * Data-Oriented Design entity registry with:
 * - 64-byte aligned arrays for cache efficiency
 * - Generational indices for safe entity handles
 * - FreeList for O(1) entity recycling
 * - Separated data highways for SIMD-friendly iteration
 */

#ifndef ENTITY_REGISTERY_H
#define ENTITY_REGISTERY_H

#include "cre_types.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>
#include "config.h"

// ============================================================================
// Entity Handle
// ============================================================================

/**
 * @brief Lightweight handle to reference an entity safely.
 * 
 * The generation field ensures stale handles don't access recycled entities.
 */
typedef struct Entity {
    uint32_t id;         ///< Index into the registry arrays
    uint32_t generation; ///< Generation counter for validation
} Entity;

/** Invalid entity sentinel value */
#define ENTITY_INVALID ((Entity){ .id = UINT32_MAX, .generation = 0 })

/** Check if an entity handle is valid (not the sentinel) */
#define ENTITY_IS_VALID(e) ((e).id != UINT32_MAX)

// ============================================================================
// Component Mask Bits (uint64_t masks[])
// ============================================================================
// These bits indicate which components an entity possesses.

#define COMP_NONE             (0ULL)
#define COMP_POSITION         (1ULL << 0)
#define COMP_VELOCITY         (1ULL << 1)
#define COMP_SIZE             (1ULL << 2)
#define COMP_ROTATION         (1ULL << 3)
#define COMP_SPRITE           (1ULL << 4)
#define COMP_COLOR            (1ULL << 5)
#define COMP_ANIMATION        (1ULL << 6)
#define COMP_PHYSICS          (1ULL << 7)
#define COMP_COLLISION_Circle (1ULL << 8)
#define COMP_COLLISION_AABB   (1ULL << 9)
// Reserve bits 10-31 for future component types
// Bits 32-63 available for game-specific components

// ============================================================================
// State Flags (uint64_t state_flags[])
// ============================================================================
// Bits 0-15:  Behavioral toggles (engine state)
// Bits 16-31: Collision Layer (which layer this entity belongs to)
// Bits 32-47: Collision Mask (which layers this entity collides with)
// Bits 48-63: Reserved for future use

// --- Behavioral Flags (Bits 0-15) ---
#define FLAG_ACTIVE        (1ULL << 0)  ///< Is this slot in use?
#define FLAG_VISIBLE       (1ULL << 1)  ///< Should renderer draw it?
#define FLAG_SOLID         (1ULL << 2)  ///< Participates in collision response
#define FLAG_ALWAYS_AWAKE  (1ULL << 3)  ///< Never enters sleep state
#define FLAG_SLEEPING      (1ULL << 4)  ///< Currently sleeping (skip physics)
#define FLAG_ANIMATED      (1ULL << 5)  ///< Uses animation system
#define FLAG_CULLED        (1ULL << 6)  ///< Outside camera view, skip rendering
#define FLAG_PERSISTENT    (1ULL << 7)  ///< Survives scene transitions
#define FLAG_STATIC        (1ULL << 8)  ///< Static in physics.
// Bits 8-15 reserved for future engine flags

// --- Collision Layer/Mask (64-bit version) ---
#define LAYER_SHIFT 16ULL
#define MASK_SHIFT  32ULL
#define LAYER_MASK_VAL 0xFFFFULL 
#define LAYER_BITS     (LAYER_MASK_VAL << LAYER_SHIFT)
#define MASK_BITS      (LAYER_MASK_VAL << MASK_SHIFT)

// Macros for collision layer/mask manipulation
#define SET_LAYER(l)     (((uint64_t)(l)) << LAYER_SHIFT)
#define GET_LAYER(flags) (((flags) >> LAYER_SHIFT) & 0xFFFFULL)
#define SET_MASK(m)      (((uint64_t)(m)) << MASK_SHIFT)
#define GET_MASK(flags)  (((flags) >> MASK_SHIFT) & 0xFFFFULL)

// Clear and set helpers
#define CLEAR_LAYER(flags) ((flags) & ~(0xFFFFULL << LAYER_SHIFT))
#define CLEAR_MASK(flags)  ((flags) & ~(0xFFFFULL << MASK_SHIFT))

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
typedef struct EntityRegistry {
    // --- Hotter Data, closer to the Top ---
    alignas(64) float pos_x[MAX_ENTITIES];              ///< Position X
    alignas(64) float pos_y[MAX_ENTITIES];              ///< Position Y
    alignas(64) float vel_x[MAX_ENTITIES];              ///< Velocity X
    alignas(64) float vel_y[MAX_ENTITIES];              ///< Velocity Y

    alignas(64) uint64_t component_masks[MAX_ENTITIES]; ///< Component presence bits
    alignas(64) uint64_t state_flags[MAX_ENTITIES];     ///< Behavioral flags + collision data
    
    alignas(64) float size_w[MAX_ENTITIES];             ///< Size width
    alignas(64) float size_h[MAX_ENTITIES];             ///< Size height
    alignas(64) uint8_t material_id[MAX_ENTITIES];      ///< Material ID for physics.
    alignas(64) float drag[MAX_ENTITIES];               ///< Air resistance etc. for physics.
    alignas(64) float inv_mass[MAX_ENTITIES];           ///< Physics Mass
    alignas(64) float gravity_scale[MAX_ENTITIES];      ///< Gravity Scales
    alignas(64) float rotation[MAX_ENTITIES];           ///< Rotation in degrees

    alignas(64) uint16_t sprite_ids[MAX_ENTITIES];      ///< Sprite/texture ID
    alignas(64) creColor colors[MAX_ENTITIES];          ///< Tint color
    alignas(64) float  pivot_x[MAX_ENTITIES];           ///< Pivot_x of sprite
    alignas(64) float  pivot_y[MAX_ENTITIES];           ///< Pivot_y of sprite

    // Animation SoA arrays - Dynamic State (managed by AnimationSystem)
    alignas(64) float    anim_timers[MAX_ENTITIES];     ///< Time accumulator for frame advance
    alignas(64) float    anim_speeds[MAX_ENTITIES];     ///< Speed multiplier (1.0f = normal)
    alignas(64) uint16_t anim_ids[MAX_ENTITIES];        ///< Current animation ID (AnimID)
    alignas(64) uint16_t anim_frames[MAX_ENTITIES];     ///< Current frame index
    alignas(64) bool     anim_finished[MAX_ENTITIES];   ///< True if non-looping anim completed

    // Animation SoA arrays - Baked Constants (copied from AnimDef on Play)
    alignas(64) float    anim_base_durations[MAX_ENTITIES]; ///< Seconds per frame (from def->defaultSpeed)
    alignas(64) uint16_t anim_frame_counts[MAX_ENTITIES];   ///< Total frames (from def->frameCount)
    alignas(64) uint16_t anim_start_sprites[MAX_ENTITIES];  ///< First sprite ID (from def->startSpriteID)
    alignas(64) bool     anim_loops[MAX_ENTITIES];          ///< Loop flag (from def->loop)

    alignas(64) uint16_t types[MAX_ENTITIES];           ///< Entity type ID
    alignas(64) uint32_t generations[MAX_ENTITIES];     ///< Generation counters
    alignas(64) uint32_t free_list[MAX_ENTITIES];       ///< Stack of free indices

    alignas(64) uint32_t free_count;                    ///< Number of free slots
    alignas(64) uint32_t active_count;                  ///< Number of active entities
    alignas(64) uint32_t max_used_bound;                ///< Highest index ever used (optimization hint)
} EntityRegistry;

#endif
