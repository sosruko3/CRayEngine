#ifndef CRE_CAMERA_UTILS_H
#define CRE_CAMERA_UTILS_H

#include "raylib.h"
// Will refactor these parts later on.

/**
 * CRE_Camera_Utils - The "Smart" Camera Toolbox
 * 
 * This module provides optional convenience functions for common camera
 * operations. Game developers can use these utilities or implement their
 * own camera behaviors using the core creCamera_* functions.
 */

/**
 * Smoothly interpolate the camera position towards a target.
 * Uses exponential decay for smooth deceleration.
 * 
 * @param target The world position to move towards
 * @param speed  Interpolation speed (higher = faster, typical range: 1.0 - 10.0)
 * @param dt     Delta time in seconds
 */
void creCamera_LerpTo(Vector2 target, float speed, float dt);

/**
 * Apply a camera shake effect.
 * Adds a randomized offset to the camera position for the current frame.
 * Call this each frame you want shake; the effect is additive.
 * 
 * @param intensity Maximum shake offset in world units
 */
void creCamera_ApplyShake(float intensity);

/**
 * Convert a screen position to world coordinates.
 * Takes into account camera position, zoom, rotation, and viewport offset.
 * 
 * @param screenPos Position in screen/pixel coordinates
 * @return Position in world coordinates
 */
Vector2 creCamera_ScreenToWorld(Vector2 screenPos);

/**
 * Convert a world position to screen coordinates.
 * Takes into account camera position, zoom, rotation, and viewport offset.
 * 
 * @param worldPos Position in world coordinates
 * @return Position in screen/pixel coordinates
 */
Vector2 creCamera_WorldToScreen(Vector2 worldPos);


// Center to the target
void creCamera_CenterOn(Vector2 targetPosition);
#endif