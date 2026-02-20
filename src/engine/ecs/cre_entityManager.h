/**
 * @file cre_entityManager.h
 * @brief Entity Manager API for Packed Parallel SoA Entity System
 * 
 * Provides O(1) entity creation and destruction with generational validation.
 * All functions take an EntityRegistry* as the first parameter.
 */

#ifndef CRE_ENTITYMANAGER_H
#define CRE_ENTITYMANAGER_H

typedef struct EntityRegistry EntityRegistry;
#include <stddef.h>
#include <stdint.h>
#include "engine/core/cre_types.h"

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
Entity EntityManager_Create(EntityRegistry* reg, int type, creVec2 pos, uint64_t initial_CompMask, uint64_t initial_flags);

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

#endif