/**
 * @file cre_physicsSystem.h
 * @brief 4-Phase Physics Pipeline for Data-Oriented Entity System
 *
 * Pipeline Architecture:
 *   Phase 0: Command Processing (CMD_PHYS_DEFINE, CMD_PHYS_LOAD_STATIC, etc.)
 *   Phase 1: Integration (Gravity, Drag, Semi-Implicit Euler)
 *   Phase 2: Broad Phase (Spatial Hash Population)
 *   Phase 3: Narrow Phase + Solver (Collision Detection & Response)
 *
 * All phases operate on EntityRegistry SoA arrays for cache efficiency.
 * Sub-stepping is configurable via PHYS_SUB_STEPS in cre_config.h.
 */

#ifndef CRE_PHYSICSSYSTEM_H
#define CRE_PHYSICSSYSTEM_H

#include <stdbool.h>
#include <stdint.h>

struct EntityRegistry;
struct CommandBus;
struct physicsPacket;

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
void PhysicsSystem_Update(physicsPacket *packet);

/**
 * @brief Process physics commands from the command bus.
 *
 * Called automatically by PhysicsSystem_Update, but can be called
 * separately for deferred command processing.
 *
 * @param reg    Pointer to the EntityRegistry
 * @param bus    Command bus to read from
 */
void PhysicsSystem_ProcessCommands(physicsPacket* packet);

physicsPacket CreatePhysicsPacket(EntityRegistry *reg, CommandBus *bus,
                                  float fixedDt);

#endif // PHYSICS_SYSTEM_H
