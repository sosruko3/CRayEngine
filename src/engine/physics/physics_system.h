/**
 * @file physics_system.h
 * @brief 4-Phase Physics Pipeline for Data-Oriented Entity System
 * 
 * Pipeline Architecture:
 *   Phase 0: Command Processing (CMD_PHYS_DEFINE, CMD_PHYS_LOAD_STATIC, etc.)
 *   Phase 1: Integration (Gravity, Drag, Semi-Implicit Euler)
 *   Phase 2: Broad Phase (Spatial Hash Population)
 *   Phase 3: Narrow Phase + Solver (Collision Detection & Response)
 * 
 * All phases operate on EntityRegistry SoA arrays for cache efficiency.
 * Sub-stepping is configurable via PHYS_SUB_STEPS in config.h.
 */

#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>
#include "physics_defs.h"

// Forward Declarations: Tells the compiler "These structs exist elsewhere"
// This prevents circular includes and speeds up compile times.
typedef struct EntityRegistry EntityRegistry;
typedef struct CommandBus CommandBus;

// ============================================================================
// Core API
// ============================================================================

/**
 * @brief Initialize the physics system.
 * 
 * Clears spatial hashes, resets gravity to defaults, logs initialization.
 * Call once at engine startup after EntityManager_Init().
 */
void PhysicsSystem_Init(void);

/**
 * @brief Main physics update - runs the complete 4-phase pipeline.
 * 
 * Executes in order:
 *   1. Phase 0: Process physics commands from bus
 *   2. Sub-step loop (PHYS_SUB_STEPS iterations):
 *      a. Phase 1: Integration (gravity, drag, movement)
 *      b. Phase 2: Broad phase (spatial hash build)
 *      c. Phase 3: Solver loop (PHYS_SOLVER_ITERATIONS):
 *         - Collision detection & response
 * 
 * @param reg    Pointer to the EntityRegistry (SoA data)
 * @param bus    Command bus for physics commands
 * @param dt     Delta time in seconds (clamped to 0.05f max internally)
 */
void PhysicsSystem_Update(EntityRegistry* reg, CommandBus* bus, float dt);

/**
 * @brief Process physics commands from the command bus.
 * 
 * Handles:
 *   - CMD_PHYS_DEFINE: Configure body mass/material from payload
 *   - CMD_PHYS_LOAD_STATIC: Populate static spatial hash
 *   - CMD_PHYS_RESET: Clear all spatial hashes
 * 
 * Called automatically by PhysicsSystem_Update, but can be called
 * separately for deferred command processing.
 * 
 * @param reg    Pointer to the EntityRegistry
 * @param bus    Command bus to read from
 */
void PhysicsSystem_ProcessCommands(EntityRegistry* reg, const CommandBus* bus);

/**
 * @brief Load static geometry into the spatial hash.
 * 
 * Scans registry for entities with:
 *   - FLAG_ACTIVE set
 *   - COMP_PHYSICS component
 *   - FLAG_STATIC set OR inv_mass <= 0
 * 
 * Adds matching entities to the static spatial hash layer.
 * Call once after scene/level loading is complete.
 * 
 * @param reg    Pointer to the EntityRegistry (const - read only)
 */
void PhysicsSystem_LoadStaticGeometry(const EntityRegistry* reg);

// ============================================================================
// Configuration API
// ============================================================================

/**
 * @brief Register or update a physics material.
 * 
 * Materials define density, friction, and restitution for collision response.
 * Predefined materials: MAT_DEFAULT, MAT_STATIC, MAT_BOUNCY, MAT_ICE.
 * 
 * @param id     Material ID (0 to PHYS_MAX_MATERIALS-1)
 * @param mat    Material properties to set
 * @return       true if successful, false if id out of range
 */
bool PhysicsSystem_SetMaterial(uint8_t id, PhysMaterial mat);

/**
 * @brief Set global gravity vector.
 * 
 * Gravity is applied during Phase 1 (Integration), scaled per-entity
 * by the gravity_scale array in EntityRegistry.
 * 
 * @param x      Gravity X component (typically 0)
 * @param y      Gravity Y component (positive = down in screen coords)
 */
void PhysicsSystem_SetGravity(float x, float y);

#endif // PHYSICS_SYSTEM_H