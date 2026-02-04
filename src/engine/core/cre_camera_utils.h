#ifndef CRE_CAMERA_UTILS_H
#define CRE_CAMERA_UTILS_H

#include "cre_types.h"
typedef struct ViewportSize ViewportSize;
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
void creCamera_LerpTo(creVec2 target, float speed, float dt);

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
creVec2 creCamera_ScreenToWorld(creVec2 screenPos,ViewportSize vp);

/**
 * Convert a world position to screen coordinates.
 * Takes into account camera position, zoom, rotation, and viewport offset.
 * 
 * @param worldPos Position in world coordinates
 * @return Position in screen/pixel coordinates
 */
creVec2 creCamera_WorldToScreen(creVec2 worldPos,ViewportSize vp);


// Center to the target
void creCamera_CenterOn(creVec2 targetPosition);
#endif