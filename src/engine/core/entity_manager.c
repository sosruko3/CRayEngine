/**
 * @file entity_manager.c
 * @brief Packed Parallel SoA Entity Manager Implementation
 */

#include "entity_manager.h"
#include "entity_registry.h"
#include "cre_types.h"
#include "cre_colors.h"

#include <stdio.h>
#include <string.h>
#include "logger.h"
#include <assert.h>

// ============================================================================
// Core API Implementation
// ============================================================================

void EntityManager_Init(EntityRegistry* reg) {
    assert(reg && "reg is NULL");
    
    // Zero everything including generations on first init
    memset(reg, 0, sizeof(EntityRegistry));
    
    // Build free list (stack: high indices at bottom, low at top)
    for (uint32_t i = 0; i < MAX_ENTITIES; i++) {
        reg->free_list[i] = (MAX_ENTITIES - 1) - i;
    }
    
    reg->free_count = MAX_ENTITIES;
    reg->active_count = 0;
    reg->max_used_bound = 0;
    
    Log(LOG_LVL_INFO, "Entity Manager Initialized (SoA, %u slots)", MAX_ENTITIES);
}

void EntityManager_Reset(EntityRegistry* reg) {
    assert(reg && "reg is NULL");
    
    // Clear component_masks and state_flags, but NOT generations!
    memset(reg->component_masks, 0, sizeof(reg->component_masks));
    memset(reg->state_flags, 0, sizeof(reg->state_flags));
    
    // Clear data highways
    memset(reg->pos_x,               0, sizeof(reg->pos_x));
    memset(reg->pos_y,               0, sizeof(reg->pos_y));
    memset(reg->vel_x,               0, sizeof(reg->vel_x));
    memset(reg->vel_y,               0, sizeof(reg->vel_y));
    memset(reg->size_w,              0, sizeof(reg->size_w));
    memset(reg->size_h,              0, sizeof(reg->size_h));

    memset(reg->inv_mass,            0, sizeof(reg->inv_mass));
    memset(reg->drag,                0, sizeof(reg->drag));
    memset(reg->gravity_scale,       0, sizeof(reg->gravity_scale));
    memset(reg->material_id,         0, sizeof(reg->material_id));

    memset(reg->rotation,            0, sizeof(reg->rotation));
    memset(reg->sprite_ids,          0, sizeof(reg->sprite_ids));
    memset(reg->colors,              0, sizeof(reg->colors));
    memset(reg->types,               0, sizeof(reg->types));

    memset(reg->anim_timers,         0, sizeof(reg->anim_timers));
    memset(reg->anim_speeds,         0, sizeof(reg->anim_speeds));
    memset(reg->anim_ids,            0, sizeof(reg->anim_ids));
    memset(reg->anim_frames,         0, sizeof(reg->anim_frames));
    memset(reg->anim_finished,       0, sizeof(reg->anim_finished));
    memset(reg->anim_base_durations, 0, sizeof(reg->anim_base_durations));

    // Rebuild free list
    for (uint32_t i = 0; i < MAX_ENTITIES; i++) {
        reg->free_list[i] = (MAX_ENTITIES - 1) - i;
    }
    
    reg->free_count = MAX_ENTITIES;
    reg->active_count = 0;
    reg->max_used_bound = 0;
    
    Log(LOG_LVL_INFO, "Entity Manager Reset Complete (generations preserved)");
}
Entity EntityManager_Create(EntityRegistry* reg, int type, creVec2 pos, uint64_t initial_CompMask, uint64_t initial_flags) {
    assert(reg && "reg is NULL");
    if (reg->free_count == 0) {
        // Registry is null or full
        return ENTITY_INVALID;
    }
    
    // Pop index from free list
    uint32_t index = reg->free_list[--reg->free_count];
    uint32_t gen = reg->generations[index];
    
    // Set up the entity in SoA arrays
    reg->component_masks[index] = initial_CompMask;
    reg->state_flags[index] = initial_flags | FLAG_ACTIVE;
    reg->types[index] = (uint16_t)type;
    
    // Position
    reg->pos_x[index] = pos.x;
    reg->pos_y[index] = pos.y;
    
    // Velocity (default zero)
    reg->vel_x[index] = 0.0f;
    reg->vel_y[index] = 0.0f;
    
    // Size (default 64x64)
    reg->size_w[index] = 64.0f;
    reg->size_h[index] = 64.0f;

    // Physics specific
    reg->inv_mass[index]      = 0.0f;
    reg->drag[index]          = 0.0f;
    reg->gravity_scale[index] = 0.0f;
    reg->material_id[index]   = 0;

    // Rotation
    reg->rotation[index] = 0.0f;
    
    // Sprite specific
    reg->sprite_ids[index] = 0;
    reg->colors[index] = creBLANK;
    reg->pivot_x[index] = 0.5f;
    reg->pivot_y[index] = 0.5f;
    
    // Animations
    reg->anim_speeds[index] = 1.0f;
    reg->anim_timers[index] = 0.0f;
    reg->anim_finished[index] = false;
    
    reg->active_count++;
    
    // Track max used index for loop optimization
    if (index >= reg->max_used_bound) {
        reg->max_used_bound = index + 1;
    }
    
    return (Entity){ .id = index, .generation = gen };
}

void EntityManager_Destroy(EntityRegistry* reg, Entity e) {
    assert(reg && "reg is NULL");
    
    // Validate handle
    if (e.id >= MAX_ENTITIES) return;
    if (!(reg->state_flags[e.id] & FLAG_ACTIVE)) return;
    if (reg->generations[e.id] != e.generation) return;
    
    // Clear the slot
    reg->component_masks[e.id] = COMP_NONE;
    reg->state_flags[e.id] = 0;
    
    // Increment generation to invalidate stale handles
    reg->generations[e.id]++;
    
    // Return slot to free list
    reg->free_list[reg->free_count++] = e.id;
    reg->active_count--;
    
    // Note: We don't shrink max_used_bound here for simplicity.
    // A more sophisticated implementation could track this.
}

void EntityManager_Shutdown(EntityRegistry* reg) {
    assert(reg && "reg is NULL");
    memset(reg, 0, sizeof(EntityRegistry));
    Log(LOG_LVL_INFO, "Entity Manager Shutdown");
}
