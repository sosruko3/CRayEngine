/**
 * @file cre_animationSystem.h
 * @brief Pure SoA Animation System with Baked Data
 * 
 * Data-Oriented animation processing using "baked" constant data.
 * All animation metadata is copied into EntityRegistry arrays when
 * an animation starts, eliminating cache misses in the hot loop.
 * 
 * Registry Arrays (Dynamic State):
 *   - anim_timers[]        : float    - time accumulator
 *   - anim_speeds[]        : float    - speed multiplier (1.0f = normal)
 *   - anim_ids[]           : uint16_t - current AnimID (for debugging/queries)
 *   - anim_frames[]        : uint16_t - current frame index
 *   - anim_finished[]      : bool     - true if non-looping anim completed
 * 
 * Registry Arrays (Baked Constants - copied on Play):
 *   - anim_base_durations[]: float    - seconds per frame
 *   - anim_frame_counts[]  : uint16_t - total frames in animation
 *   - anim_start_sprites[] : uint16_t - first sprite ID
 *   - anim_loops[]         : bool     - whether animation loops
 */

#ifndef CRE_ANIMATIONSYSTEM_H
#define CRE_ANIMATIONSYSTEM_H

#include <stdint.h>
#include <stdbool.h>

// Forward declarations (dependency injection)
typedef struct EntityRegistry EntityRegistry;
typedef struct CommandBus CommandBus;

/**
 * @brief Process animation commands from the command bus.
 * 
 * Handles CMD_ANIM_PLAY, CMD_ANIM_STOP, CMD_ANIM_PAUSE etc. commands.
 * Uses switch dispatch for command types.
 * 
 * @param reg Pointer to EntityRegistry (SoA data)
 * @param bus Pointer to CommandBus (read-only iteration)
 */
void AnimationSystem_ProcessCommands(EntityRegistry* reg, CommandBus* bus);

/**
 * @brief Advance animation state for all active animated entities.
 * 
 * Pure SoA hot loop - reads ONLY from registry arrays, never from ASSET_ANIMS.
 * Writes sprite_ids[] only when frame changes and entity is visible.
 * 
 * @param reg Pointer to EntityRegistry
 * @param bus Pointer to CommandBus
 * @param dt Delta time in seconds (clamped to 0.05f max internally)
 */
void AnimationSystem_Update(EntityRegistry* reg,CommandBus* bus, float dt);

/**
 * @brief Set animation speed multiplier for an entity.
 * 
 * @param reg Pointer to EntityRegistry
 * @param entityID Entity index
 * @param multiplier Speed multiplier (1.0f = normal, 2.0f = double speed)
 */
void AnimationSystem_SetSpeed(EntityRegistry* reg, uint32_t entityID, float multiplier);

/**
 * @brief Play an animation on an entity (direct call, bypasses command bus).
 * 
 * "The Baker" - looks up ASSET_ANIMS[animID] and copies constant data
 * (duration, frameCount, startSprite, loop) into the registry's baked arrays.
 * 
 * @param reg Pointer to EntityRegistry
 * @param entityID Entity index
 * @param animID Animation ID to play
 * @param forceReset If true, restarts animation even if already playing
 */
void AnimationSystem_Play(EntityRegistry* reg, uint32_t entityID, uint16_t animID, bool forceReset);

#endif