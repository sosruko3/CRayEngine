/**
 * @file entity_manager.c
 * @brief Packed Parallel SoA Entity Manager Implementation
 */

#include "entity_manager.h"
#include <stdio.h>
#include <string.h>
#include "logger.h"

// ============================================================================
// Static Registry (The Single Source of Truth)
// ============================================================================

static EntityRegistry s_registry;

// ============================================================================
// Core API Implementation
// ============================================================================

void EntityManager_Init(void) {
    // Zero everything including generations on first init
    memset(&s_registry, 0, sizeof(s_registry));
    
    // Build free list (stack: high indices at bottom, low at top)
    for (uint32_t i = 0; i < MAX_ENTITIES; i++) {
        s_registry.free_list[i] = (MAX_ENTITIES - 1) - i;
    }
    
    s_registry.free_count = MAX_ENTITIES;
    s_registry.active_count = 0;
    s_registry.max_used_bound = 0;
    
    Log(LOG_LVL_INFO, "Entity Manager Initialized (SoA, %u slots)", MAX_ENTITIES);
}

void EntityManager_Reset(void) {
    // Clear component_masks and state_flags, but NOT generations!
    memset(s_registry.component_masks, 0, sizeof(s_registry.component_masks));
    memset(s_registry.state_flags, 0, sizeof(s_registry.state_flags));
    
    // Clear data highways
    memset(s_registry.pos_x, 0, sizeof(s_registry.pos_x));
    memset(s_registry.pos_y, 0, sizeof(s_registry.pos_y));
    memset(s_registry.vel_x, 0, sizeof(s_registry.vel_x));
    memset(s_registry.vel_y, 0, sizeof(s_registry.vel_y));
    memset(s_registry.size_w, 0, sizeof(s_registry.size_w));
    memset(s_registry.size_h, 0, sizeof(s_registry.size_h));

    memset(s_registry.rotation, 0, sizeof(s_registry.rotation));
    memset(s_registry.sprite_ids, 0, sizeof(s_registry.sprite_ids));
    memset(s_registry.colors, 0, sizeof(s_registry.colors));
    memset(s_registry.types, 0, sizeof(s_registry.types));

    memset(s_registry.anim_timers, 0, sizeof(s_registry.anim_timers));
    memset(s_registry.anim_speeds, 0, sizeof(s_registry.anim_speeds));
    memset(s_registry.anim_ids,    0, sizeof(s_registry.anim_ids));
    memset(s_registry.anim_frames, 0, sizeof(s_registry.anim_frames));
    memset(s_registry.anim_finished, 0, sizeof(s_registry.anim_finished));
    memset(s_registry.anim_base_durations, 0, sizeof(s_registry.anim_base_durations));

    // Rebuild free list
    for (uint32_t i = 0; i < MAX_ENTITIES; i++) {
        s_registry.free_list[i] = (MAX_ENTITIES - 1) - i;
    }
    
    s_registry.free_count = MAX_ENTITIES;
    s_registry.active_count = 0;
    s_registry.max_used_bound = 0;
    
    Log(LOG_LVL_INFO, "Entity Manager Reset Complete (generations preserved)");
}

Entity EntityManager_Create(int type, Vector2 pos,uint64_t initial_CompMask,uint64_t initial_flags) {
    if (s_registry.free_count == 0) {
        // Registry is full
        return ENTITY_INVALID;
    }
    
    // Pop index from free list
    uint32_t index = s_registry.free_list[--s_registry.free_count];
    uint32_t gen = s_registry.generations[index];
    
    // Set up the entity in SoA arrays
    s_registry.component_masks[index] = initial_CompMask;
    s_registry.state_flags[index] = initial_flags | FLAG_ACTIVE;
    s_registry.types[index] = (uint16_t)type;
    
    // Position
    s_registry.pos_x[index] = pos.x;
    s_registry.pos_y[index] = pos.y;
    
    // Velocity (default zero)
    s_registry.vel_x[index] = 0.0f;
    s_registry.vel_y[index] = 0.0f;
    
    // Size (default 32x32)
    s_registry.size_w[index] = 32.0f;
    s_registry.size_h[index] = 32.0f;
    
    // Rotation
    s_registry.rotation[index] = 0.0f;
    
    // Sprite and color
    s_registry.sprite_ids[index] = 0;
    s_registry.colors[index] = WHITE;
    // Animations
    s_registry.anim_speeds[index] = 1.0f;
    s_registry.anim_timers[index] = 0.0f;
    s_registry.anim_finished[index] = false;
    
    s_registry.active_count++;
    
    // Track max used index for loop optimization
    if (index >= s_registry.max_used_bound) {
        s_registry.max_used_bound = index + 1;
    }
    
    return (Entity){ .id = index, .generation = gen };
}

void EntityManager_Destroy(Entity e) {
    // Validate handle
    if (e.id >= MAX_ENTITIES) return;
    if (!(s_registry.state_flags[e.id] & FLAG_ACTIVE)) return;
    if (s_registry.generations[e.id] != e.generation) return;
    
    // Clear the slot
    s_registry.component_masks[e.id] = COMP_NONE;
    s_registry.state_flags[e.id] = 0;
    
    // Increment generation to invalidate stale handles
    s_registry.generations[e.id]++;
    
    // Return slot to free list
    s_registry.free_list[s_registry.free_count++] = e.id;
    s_registry.active_count--;
    
    // Note: We don't shrink max_used_bound here for simplicity.
    // A more sophisticated implementation could track this.
}

void EntityManager_Shutdown(void) {
    memset(&s_registry, 0, sizeof(s_registry));
    Log(LOG_LVL_INFO, "Entity Manager Shutdown");
}

EntityRegistry* EntityManager_GetRegistry(void) {
    return &s_registry;
}

uint32_t EntityManager_GetActiveCount(void) {
    return s_registry.active_count;
}

uint32_t EntityManager_GetMaxUsedBound(void) {
    return s_registry.max_used_bound;
}

// ============================================================================
// Legacy Compatibility Implementation
// ============================================================================

bool EntityManager_GetLegacy(Entity e, EntityData* out_data) {
    if (!EntityManager_IsValid(e) || out_data == NULL) return false;
    
    uint32_t id = e.id;
    
    out_data->position = (Vector2){ s_registry.pos_x[id], s_registry.pos_y[id] };
    out_data->velocity = (Vector2){ s_registry.vel_x[id], s_registry.vel_y[id] };
    out_data->size = (Vector2){ s_registry.size_w[id], s_registry.size_h[id] };
    out_data->color = s_registry.colors[id];
    out_data->rotation = s_registry.rotation[id];
    out_data->restitution = 0.0f; // Not stored in SoA
    out_data->generation = s_registry.generations[id];
    out_data->flags = (uint32_t)(s_registry.state_flags[id] & 0xFFFFFFFF);
    out_data->spriteID = s_registry.sprite_ids[id];
    out_data->type = s_registry.types[id];
    
    return true;
}

bool EntityManager_SetLegacy(Entity e, const EntityData* data) {
    if (!EntityManager_IsValid(e) || data == NULL) return false;
    
    uint32_t id = e.id;
    
    s_registry.pos_x[id] = data->position.x;
    s_registry.pos_y[id] = data->position.y;
    s_registry.vel_x[id] = data->velocity.x;
    s_registry.vel_y[id] = data->velocity.y;
    s_registry.size_w[id] = data->size.x;
    s_registry.size_h[id] = data->size.y;
    s_registry.colors[id] = data->color;
    s_registry.rotation[id] = data->rotation;
    s_registry.state_flags[id] = (s_registry.state_flags[id] & 0xFFFFFFFF00000000ULL) | data->flags;
    s_registry.sprite_ids[id] = data->spriteID;
    s_registry.types[id] = data->type;
    
    return true;
}