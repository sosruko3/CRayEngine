/**
 * @file cre_entityManager.c
 * @brief Packed Parallel SoA Entity Manager Implementation
 */

#include "cre_entityManager.h"
#include "cre_entityRegistry.h"
#include "engine/core/cre_colors.h"
#include "engine/core/cre_logger.h"
#include "engine/core/cre_types.h"
#include <assert.h>
#include <string.h>

// ============================================================================
// Core API Implementation
// ============================================================================

void EntityManager_Init(EntityRegistry &reg) {

  // Zero everything including generations on first init
  memset(&reg, 0, sizeof(EntityRegistry));

  // Build free list (stack: high indices at bottom, low at top)
  for (uint32_t i = 0; i < MAX_ENTITIES; i++) {
    reg.free_list[i] = (MAX_ENTITIES - 1) - i;
  }

  reg.free_count = MAX_ENTITIES;
  reg.active_count = 0;
  reg.max_used_bound = 0;

  Log(LogLevel::Info, "Entity Manager Initialized (SoA, {} slots)",
      MAX_ENTITIES);
}

void EntityManager_Reset(EntityRegistry &reg) {

  // Clear component_masks and state_flags, but NOT generations!
  memset(reg.component_masks, 0, sizeof(reg.component_masks));
  memset(reg.state_flags, 0, sizeof(reg.state_flags));
  memset(reg.render_layer, 0, sizeof(reg.render_layer));
  memset(reg.batch_ids, 0, sizeof(reg.batch_ids));

  // Clear data highways
  memset(reg.pos, 0, sizeof(reg.pos));
  memset(reg.vel, 0, sizeof(reg.vel));
  memset(reg.size, 0, sizeof(reg.size));

  memset(reg.inv_mass, 0, sizeof(reg.inv_mass));
  memset(reg.drag, 0, sizeof(reg.drag));
  memset(reg.gravity_scale, 0, sizeof(reg.gravity_scale));
  memset(reg.material_id, 0, sizeof(reg.material_id));

  memset(reg.rotation, 0, sizeof(reg.rotation));
  memset(reg.sprite_ids, 0, sizeof(reg.sprite_ids));
  memset(reg.colors, 0, sizeof(reg.colors));
  memset(reg.types, 0, sizeof(reg.types));
  memset(reg.visual_scale, 0, sizeof(reg.visual_scale));

  memset(reg.anim_timers, 0, sizeof(reg.anim_timers));
  memset(reg.anim_speeds, 0, sizeof(reg.anim_speeds));
  memset(reg.anim_ids, 0, sizeof(reg.anim_ids));
  memset(reg.anim_frames, 0, sizeof(reg.anim_frames));
  memset(reg.anim_finished, 0, sizeof(reg.anim_finished));
  memset(reg.anim_base_durations, 0, sizeof(reg.anim_base_durations));
  memset(reg.cameras, 0, sizeof(reg.cameras));
  reg.camera_count = 0;

  memset(&reg.events, 0, sizeof(EntityEventDispatcher));

  // Rebuild free list
  for (uint32_t i = 0; i < MAX_ENTITIES; i++) {
    reg.free_list[i] = (MAX_ENTITIES - 1) - i;
  }

  reg.free_count = MAX_ENTITIES;
  reg.active_count = 0;
  reg.max_used_bound = 0;

  Log(LogLevel::Info, "Entity Manager Reset Complete (generations preserved)");
}

Entity EntityManager_ReserveSlot(EntityRegistry &reg) {

  if (reg.free_count == 0)
    return ENTITY_INVALID;

  const uint32_t index = reg.free_list[--reg.free_count];
  const uint32_t generation = reg.generations[index];

  return Entity{.id = index, .generation = generation};
}

void EntityManager_ReturnReservedSlot(EntityRegistry &reg,
                                      Entity reserved_entity) {

  if (reserved_entity.id >= MAX_ENTITIES)
    return;

  assert(reg.free_count < MAX_ENTITIES &&
         "Double return / Free list overflow!");

  reg.free_list[reg.free_count++] = reserved_entity.id;
  reg.generations[reserved_entity.id]++;
}

Entity EntityManager_Create(EntityRegistry &reg, uint16_t type, creVec2 pos,
                            uint64_t initial_CompMask, uint64_t initial_flags) {
  if (reg.free_count == 0)
    return ENTITY_INVALID;

  // Pop index from free list
  uint32_t index = reg.free_list[--reg.free_count];
  uint32_t gen = reg.generations[index];

  // Set up the entity in SoA arrays
  reg.component_masks[index] = initial_CompMask;
  reg.state_flags[index] = initial_flags;
  reg.types[index] = static_cast<uint16_t>(type);
  reg.render_layer[index] = 0; // Keep these in mind
  reg.batch_ids[index] = 0;    // **

  // Position
  reg.pos[index] = pos;

  // Velocity (default zero)
  reg.vel[index] = creVec2{0.0f, 0.0f};

  // Size (default 64x64)
  reg.size[index] = creVec2{64.0f, 64.0f};

  // Physics specific
  reg.inv_mass[index] = 0.0f;
  reg.drag[index] = 0.0f;
  reg.gravity_scale[index] = 0.0f;
  reg.material_id[index] = 0;

  // Rotation
  reg.rotation[index] = 0.0f;

  // Sprite specific
  reg.sprite_ids[index] = 0;
  reg.colors[index] = creBLANK;
  reg.pivot[index] = creVec2{0.5f, 0.5f};
  reg.visual_scale[index] = creVec2{1.0f, 1.0f};

  // Animations
  reg.anim_speeds[index] = 1.0f;
  reg.anim_timers[index] = 0.0f;
  reg.anim_finished[index] = false;

  reg.active_count++;

  // Track max used index for loop optimization
  if (index >= reg.max_used_bound)
    reg.max_used_bound = index + 1;

  return Entity{.id = index, .generation = gen};
}

void EntityManager_Destroy(EntityRegistry &reg, Entity e) {

  // Validate handle
  if (e.id >= MAX_ENTITIES)
    return;
  if (!(reg.state_flags[e.id] & FLAG_ACTIVE))
    return;
  if (reg.generations[e.id] != e.generation)
    return;

  // Clear the slot
  reg.component_masks[e.id] = COMP_NONE;
  reg.state_flags[e.id] = 0;
  reg.render_layer[e.id] = 0; // Keep these in mind**
  reg.batch_ids[e.id] = 0;

  // Increment generation to invalidate stale handles
  reg.generations[e.id]++;

  // Return slot to free list
  reg.free_list[reg.free_count++] = e.id;
  reg.active_count--;

  // Note: We don't shrink max_used_bound here for simplicity.
  // A more sophisticated implementation could track this.
}

void EntityManager_Shutdown(EntityRegistry &reg) {
  memset(&reg, 0, sizeof(EntityRegistry));
  Log(LogLevel::Info, "Entity Manager Shutdown");
}
