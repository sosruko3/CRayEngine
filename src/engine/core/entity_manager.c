/**
 * @file entity_manager.c
 * @brief Packed Parallel SoA Entity Manager Implementation
 */

#include "entity_manager.h"
#include <stdio.h>
#include <string.h>
#include "logger.h"

// ============================================================================
// Core API Implementation
// ============================================================================

void EntityManager_Init(EntityRegistry* reg) {
    if (!reg) return;
    
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
    if (!reg) return;
    
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

Entity EntityManager_Create(EntityRegistry* reg, int type, Vector2 pos, uint64_t initial_CompMask, uint64_t initial_flags) {
    if (!reg || reg->free_count == 0) {
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
    
    // Size (default 32x32)
    reg->size_w[index] = 32.0f;
    reg->size_h[index] = 32.0f;

    // Physics specific
    reg->inv_mass[index]      = 0.0f;
    reg->drag[index]          = 0.0f;
    reg->gravity_scale[index] = 0.0f;
    reg->material_id[index]   = 0;

    // Rotation
    reg->rotation[index] = 0.0f;
    
    // Sprite and color
    reg->sprite_ids[index] = 0;
    reg->colors[index] = WHITE;
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
    if (!reg) return;
    
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
    if (!reg) return;
    memset(reg, 0, sizeof(EntityRegistry));
    Log(LOG_LVL_INFO, "Entity Manager Shutdown");
}

// ============================================================================
// Legacy Compatibility Implementation
// ============================================================================

bool EntityManager_GetLegacy(EntityRegistry* reg, Entity e, EntityData* out_data) {
    if (!reg || !EntityManager_IsValid(reg, e) || out_data == NULL) return false;
    
    uint32_t id = e.id;
    
    out_data->position = (Vector2){ reg->pos_x[id], reg->pos_y[id] };
    out_data->velocity = (Vector2){ reg->vel_x[id], reg->vel_y[id] };
    out_data->size = (Vector2){ reg->size_w[id], reg->size_h[id] };
    out_data->color = reg->colors[id];
    out_data->rotation = reg->rotation[id];
    out_data->restitution = 0.0f; // Not stored in SoA
    out_data->generation = reg->generations[id];
    out_data->flags = (uint32_t)(reg->state_flags[id] & 0xFFFFFFFF);
    out_data->spriteID = reg->sprite_ids[id];
    out_data->type = reg->types[id];
    
    return true;
}

bool EntityManager_SetLegacy(EntityRegistry* reg, Entity e, const EntityData* data) {
    if (!reg || !EntityManager_IsValid(reg, e) || data == NULL) return false;
    
    uint32_t id = e.id;
    
    reg->pos_x[id] = data->position.x;
    reg->pos_y[id] = data->position.y;
    reg->vel_x[id] = data->velocity.x;
    reg->vel_y[id] = data->velocity.y;
    reg->size_w[id] = data->size.x;
    reg->size_h[id] = data->size.y;
    reg->colors[id] = data->color;
    reg->rotation[id] = data->rotation;
    reg->state_flags[id] = (reg->state_flags[id] & 0xFFFFFFFF00000000ULL) | data->flags;
    reg->sprite_ids[id] = data->spriteID;
    reg->types[id] = data->type;
    
    return true;
}