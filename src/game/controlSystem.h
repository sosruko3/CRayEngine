#ifndef CONTROLSYSTEM_H
#define CONTROLSYSTEM_H

#include <stdint.h>
#include "engine/core/cre_types.h"
typedef struct EntityRegistry EntityRegistry;
typedef struct CommandBus CommandBus;
typedef struct CameraComponent CameraComponent;
/**
 * @brief Update entity logic based on input (player movement, etc.)
 * @param reg Pointer to the EntityRegistry
 * @param dt Delta time in seconds
 */
void ControlSystem_UpdateLogic(EntityRegistry* reg, float dt, creRectangle cullBounds);

/**
 * @brief Handle camera zoom input (primary/secondary actions).
 * @param dt Delta time in seconds
 */
void ControlSystem_ChangeZoom(EntityRegistry* reg, CommandBus* bus, float dt);

/**
 * @brief Set which entity the camera should follow.
 * @param reg Pointer to the EntityRegistry
 * @param target Entity handle to follow
 */
void ControlSystem_SetCameraTarget(EntityRegistry* reg, CommandBus* bus, Entity target,Entity camEntity);

/**
 * @brief Update sleep state for entities based on distance from camera target.
 * 
 * Entities far from the camera target are marked as FLAG_CULLED.
 * @param reg Pointer to the EntityRegistry
 */
void ControlSystem_UpdateSleepState(EntityRegistry* reg,const CameraComponent* cam);

/**
 * @brief Handle debug entity spawning input (Z batch, X single).
 * @param reg Pointer to the EntityRegistry
 * @param bus Command bus for physics setup commands
 */
void ControlSystem_HandleDebugSpawning(EntityRegistry* reg, CommandBus* bus);

/**
 * @brief Spawn the player entity at default position.
 * @param reg Pointer to the EntityRegistry
 */
Entity ControlSystem_SpawnPlayer(EntityRegistry* reg, CommandBus* bus);

Entity ControlSystem_SpawnCamera(EntityRegistry *reg);

#endif