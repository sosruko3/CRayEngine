/**
 * @file entity_manager.h
 * @brief Entity Manager API for Packed Parallel SoA Entity System
 * 
 * Provides O(1) entity creation and destruction with generational validation.
 * All functions take an EntityRegistry* as the first parameter.
 */

#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include "entity_registry.h"
#include <stddef.h>

// ============================================================================
// Core API
// ============================================================================

/**
 * @brief Initialize the entity manager. Call once at engine startup.
 * @param reg Pointer to the EntityRegistry to initialize
 */
void EntityManager_Init(EntityRegistry* reg);

/**
 * @brief Reset all entities without reallocating memory.
 * 
 * Clears all component_masks and state_flags to 0, rebuilds the FreeList.
 * Does NOT reset generations array - old handles remain invalid.
 * @param reg Pointer to the EntityRegistry to reset
 */
void EntityManager_Reset(EntityRegistry* reg);

/**
 * @brief Create a new entity with the given type and position.
 * 
 * @param reg Pointer to the EntityRegistry
 * @param type Entity type identifier
 * @param pos Initial world position
 * @param initial_CompMask Initial component mask
 * @param initial_flags Initial state flags
 * @return Entity handle, or ENTITY_INVALID if registry is full
 */
Entity EntityManager_Create(EntityRegistry* reg, int type, Vector2 pos, uint64_t initial_CompMask, uint64_t initial_flags);

/**
 * @brief Destroy an entity, returning its slot to the free list.
 * 
 * Increments generation to invalidate stale handles.
 * 
 * @param reg Pointer to the EntityRegistry
 * @param e Entity handle to destroy
 */
void EntityManager_Destroy(EntityRegistry* reg, Entity e);

/**
 * @brief Shutdown the entity manager. Call at engine shutdown.
 * @param reg Pointer to the EntityRegistry to shutdown
 */
void EntityManager_Shutdown(EntityRegistry* reg);

// ============================================================================
// Validation & Access Helpers
// ============================================================================

/**
 * @brief Check if an entity handle is still valid.
 * 
 * Validates that:
 * - ID is within bounds
 * - Entity is active
 * - Generation matches
 * 
 * @param reg Pointer to the EntityRegistry
 * @param e Entity handle to validate
 * @return true if valid, false otherwise
 */
static inline bool EntityManager_IsValid(EntityRegistry* reg, Entity e) {
    if (!reg || e.id >= MAX_ENTITIES) return false;
    if (!(reg->state_flags[e.id] & FLAG_ACTIVE)) return false;
    if (reg->generations[e.id] != e.generation) return false;
    return true;
}

/**
 * @brief Get entity state flags (includes behavioral flags + collision layer/mask).
 * 
 * @param reg Pointer to the EntityRegistry
 * @param e Entity handle
 * @return Pointer to state_flags, or NULL if invalid
 */
static inline uint64_t* EntityManager_GetStateFlags(EntityRegistry* reg, Entity e) {
    if (!EntityManager_IsValid(reg, e)) return NULL;
    return &reg->state_flags[e.id];
}

/**
 * @brief Get entity component mask.
 * 
 * @param reg Pointer to the EntityRegistry
 * @param e Entity handle
 * @return Pointer to component mask, or NULL if invalid
 */
static inline uint64_t* EntityManager_GetCompMask(EntityRegistry* reg, Entity e) {
    if (!EntityManager_IsValid(reg, e)) return NULL;
    return &reg->component_masks[e.id];
}

// ============================================================================
// Legacy Compatibility (for gradual migration)
// ============================================================================

/**
 * @brief Legacy compatibility - get EntityData view for an entity.
 * @deprecated Use direct SoA access for new code.
 * 
 * This creates a COPY of the data. For performance-critical code,
 * access the registry arrays directly.
 * 
 * @param reg Pointer to the EntityRegistry
 * @param e Entity handle
 * @param out_data Output struct to fill
 * @return true if entity is valid and data was copied, false otherwise
 */
bool EntityManager_GetLegacy(EntityRegistry* reg, Entity e, EntityData* out_data);

/**
 * @brief Legacy compatibility - set EntityData for an entity.
 * @deprecated Use direct SoA access for new code.
 * 
 * @param reg Pointer to the EntityRegistry
 * @param e Entity handle
 * @param data Data to copy into the registry
 * @return true if entity is valid and data was set, false otherwise
 */
bool EntityManager_SetLegacy(EntityRegistry* reg, Entity e, const EntityData* data);

#endif // ENTITY_MANAGER_H