#ifndef CONTROLSYSTEM_H
#define CONTROLSYSTEM_H

#include <stdint.h>
typedef struct EntityRegistry EntityRegistry;
typedef struct CommandBus CommandBus;

/**
 * @brief Update entity logic based on input (player movement, etc.)
 * @param reg Pointer to the EntityRegistry
 * @param dt Delta time in seconds
 */
void ControlSystem_UpdateLogic(EntityRegistry* reg, float dt);

/**
 * @brief Handle camera zoom input (primary/secondary actions).
 */
void ControlSystem_ChangeZoom(void);

/**
 * @brief Set which entity the camera should follow.
 * @param reg Pointer to the EntityRegistry
 * @param entityID Entity index to follow
 */
void ControlSystem_SetCameraTarget(EntityRegistry* reg, uint32_t entityID);

/**
 * @brief Update sleep state for entities based on distance from camera target.
 * 
 * Entities far from the camera target are marked as FLAG_CULLED.
 * @param reg Pointer to the EntityRegistry
 */
void ControlSystem_UpdateSleepState(EntityRegistry* reg);

/**
 * @brief Spawn the player entity at default position.
 * @param reg Pointer to the EntityRegistry
 */
void ControlSystem_SpawnPlayer(EntityRegistry* reg, CommandBus* bus);

#endif