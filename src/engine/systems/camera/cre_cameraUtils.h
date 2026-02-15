#ifndef CRE_CAMERAUTILS_H
#define CRE_CAMERAUTILS_H

#include "engine/core/cre_types.h"
typedef struct Camera2D Camera2D;

/**
 * CRE_Camera_Utils - The "Smart" Camera Toolbox
 * 
 * Math and conversion helpers used by camera systems.
 * No camera state is stored or mutated in this module.
 */

/**
 * Smoothly interpolate a position towards a target.
 * Uses exponential decay for smooth deceleration.
 * 
 * @param current Current position
 * @param target The world position to move towards
 * @param speed  Interpolation speed (higher = faster, typical range: 1.0 - 10.0)
 * @param dt     Delta time in seconds
 * @return Interpolated position
 */
creVec2 cameraUtils_Lerp(creVec2 current, creVec2 target, float speed, float dt);

/**
 * Create randomized camera shake offset.
 * 
 * @param intensity Maximum shake offset in world units
 * @return Offset vector to add to camera target
 */
creVec2 cameraUtils_RandomShakeOffset(float intensity);

/**
 * Convert a screen position to world coordinates.
 * Takes into account camera position, zoom, rotation, and viewport offset.
 * 
 * @param screenPos Position in screen/pixel coordinates
 * @param cam Active camera transform
 * @return Position in world coordinates
 */
creVec2 cameraUtils_ScreenToWorld(creVec2 screenPos, Camera2D cam);

/**
 * Convert a world position to screen coordinates.
 * Takes into account camera position, zoom, rotation, and viewport offset.
 * 
 * @param worldPos Position in world coordinates
 * @param cam Active camera transform
 * @return Position in screen/pixel coordinates
 */
creVec2 cameraUtils_WorldToScreen(creVec2 worldPos, Camera2D cam);

#endif