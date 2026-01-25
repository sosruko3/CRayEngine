#ifndef CRE_CAMERA_H
#define CRE_CAMERA_H

#include "raylib.h"
#include "viewport.h" // REMOVE THIS AFTER IMPLEMENTING SERVICE LOCATOR

/**
 * CRE_Camera - The "Stupid" Core Camera
 * 
 * This struct holds the logical camera state in world space.
 * The engine uses this to construct a Raylib Camera2D with proper
 * viewport-aware centering.
 */
typedef struct {
    Vector2 position;   // Camera target position in world space
    float   zoom;       // Zoom level (1.0 = default)
    float   rotation;   // Rotation in degrees
} CRE_Camera;

void creCamera_UpdateViewportCache(ViewportSize vp);

/**
 * Initialize the camera system with default values.
 * Call this once during engine initialization.
 */
void creCamera_Init(ViewportSize vp);

/**
 * Set the camera's world position directly.
 */
void creCamera_SetPosition(Vector2 position);

/**
 * Get the camera's current world position.
 */
Vector2 creCamera_GetPosition(void);

/**
 * Set the camera zoom level.
 * @param zoom Zoom multiplier (1.0 = normal, 2.0 = 2x zoom in, 0.5 = zoom out)
 */
void creCamera_SetZoom(float zoom);

/**
 * Get the camera's current zoom level.
 */
float creCamera_GetZoom(void);

/**
 * Set the camera rotation.
 * @param rotation Rotation in degrees
 */
void creCamera_SetRotation(float rotation);

/**
 * Get the camera's current rotation.
 */
float creCamera_GetRotation(void);

/**
 * Get the internal Raylib Camera2D for rendering.
 * The offset is automatically calculated from Viewport_Get() to ensure
 * the camera target is always centered regardless of window size.
 */
Camera2D creCamera_GetInternal(ViewportSize vp);

/**
 * Get the visible area in world space for frustum culling.
 * @return Rectangle representing the camera's view bounds in world coordinates
 */
Rectangle creCamera_GetViewBounds(ViewportSize vp);


// For Frustum Culling
Rectangle creCamera_GetCullBounds(ViewportSize vp);
#endif