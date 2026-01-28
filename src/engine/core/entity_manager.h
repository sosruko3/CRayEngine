/**
 * @file entity_manager.h
 * @brief Entity Manager API for Packed Parallel SoA Entity System
 * 
 * Provides O(1) entity creation and destruction with generational validation.
 * All data is stored in a static EntityRegistry accessible via EntityManager_GetRegistry().
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
 */
void EntityManager_Init(void);

/**
 * @brief Reset all entities without reallocating memory.
 * 
 * Clears all component_masks and state_flags to 0, rebuilds the FreeList.
 * Does NOT reset generations array - old handles remain invalid.
 */
void EntityManager_Reset(void);

/**
 * @brief Create a new entity with the given type and position.
 * 
 * @param type Entity type identifier
 * @param pos Initial world position
 * @return Entity handle, or ENTITY_INVALID if registry is full
 */
Entity EntityManager_Create(int type, Vector2 pos,uint64_t initial_CompMask,uint64_t initial_flags);

/**
 * @brief Destroy an entity, returning its slot to the free list.
 * 
 * Increments generation to invalidate stale handles.
 * 
 * @param e Entity handle to destroy
 */
void EntityManager_Destroy(Entity e);

/**
 * @brief Shutdown the entity manager. Call at engine shutdown.
 */
void EntityManager_Shutdown(void);

/**
 * @brief Get a pointer to the static EntityRegistry.
 * 
 * Use this for direct SoA array access in performance-critical code.
 * 
 * @return Pointer to the EntityRegistry
 */
EntityRegistry* EntityManager_GetRegistry(void);

/**
 * @brief Get the number of currently active entities.
 * @return Number of active entities
 */
uint32_t EntityManager_GetActiveCount(void);

/**
 * @brief Get the highest index currently in use (optimization hint for loops).
 * 
 * Systems can iterate from 0 to max_used_bound instead of MAX_ENTITIES.
 * 
 * @return Maximum index that may contain an active entity
 */
uint32_t EntityManager_GetMaxUsedBound(void);

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
 * @param e Entity handle to validate
 * @return true if valid, false otherwise
 */
static inline bool EntityManager_IsValid(Entity e) {
    if (e.id >= MAX_ENTITIES) return false;
    EntityRegistry* reg = EntityManager_GetRegistry();
    if (!(reg->state_flags[e.id] & FLAG_ACTIVE)) return false;
    if (reg->generations[e.id] != e.generation) return false;
    return true;
}

/**
 * @brief Get entity state flags (includes behavioral flags + collision layer/mask).
 * 
 * @param e Entity handle
 * @return Pointer to state_flags, or NULL if invalid
 */
static inline uint64_t* EntityManager_GetStateFlags(Entity e) {
    if (!EntityManager_IsValid(e)) return NULL;
    EntityRegistry* reg = EntityManager_GetRegistry();
    return &reg->state_flags[e.id];
}

/**
 * @brief Get entity component mask.
 * 
 * @param e Entity handle
 * @return Pointer to component mask, or NULL if invalid
 */
static inline uint64_t* EntityManager_GetCompMask(Entity e) {
    if (!EntityManager_IsValid(e)) return NULL;
    EntityRegistry* reg = EntityManager_GetRegistry();
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
 * @param e Entity handle
 * @param out_data Output struct to fill
 * @return true if entity is valid and data was copied, false otherwise
 */
bool EntityManager_GetLegacy(Entity e, EntityData* out_data);

/**
 * @brief Legacy compatibility - set EntityData for an entity.
 * @deprecated Use direct SoA access for new code.
 * 
 * @param e Entity handle
 * @param data Data to copy into the registry
 * @return true if entity is valid and data was set, false otherwise
 */
bool EntityManager_SetLegacy(Entity e, const EntityData* data);

#endif // ENTITY_MANAGER_H