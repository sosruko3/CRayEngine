#include "cre_camera.h"
#include "viewport.h"
#include "raylib.h"
#include <math.h>
#include "config.h"
#include "logger.h"

// ============================================================================
// Static Internal State - The "Stupid" Core
// ============================================================================

static CRE_Camera s_camera = {
    .position = {0,0},
    .rotation = 0.0f,
    .zoom = 1.0f
};

static struct {
    CRE_Camera state;
    float baseDiagonal; // Cached diagonal of the screen (unzoomed)
} s_internal = {0};

// ============================================================================
// Helper functions
// ============================================================================

void creCamera_UpdateViewportCache(void) {
    ViewportSize vp = Viewport_Get();
    s_internal.baseDiagonal = sqrtf((vp.width * vp.width) + (vp.height * vp.height));
}
// ============================================================================
// Core Camera Functions
// ============================================================================

void creCamera_Init(void) {
    s_camera.position = (Vector2){0.0f, 0.0f};
    s_camera.zoom = 1.0f;
    s_camera.rotation = 0.0f;
    creCamera_UpdateViewportCache();
}

void creCamera_SetPosition(Vector2 position) {
    s_camera.position = position;
}

Vector2 creCamera_GetPosition(void) {
    return s_camera.position;
}

void creCamera_SetZoom(float zoom) {
    // Clamp zoom to reasonable values to prevent issues 
    if (zoom < MIN_ZOOM) zoom = MIN_ZOOM;
    if (zoom > MAX_ZOOM) zoom = MAX_ZOOM;
    s_camera.zoom = zoom;
}

float creCamera_GetZoom(void) {
    return s_camera.zoom;
}

void creCamera_SetRotation(float rotation) {
    s_camera.rotation = rotation;
}

float creCamera_GetRotation(void) {
    return s_camera.rotation;
}

Camera2D creCamera_GetInternal(void) {
    // Check for resize events
    if (Viewport_wasResized()) {
        creCamera_UpdateViewportCache();
        Log(LOG_LVL_DEBUG,"Window Resized - Camera Cache Updated");
    }
    // Get current viewport dimensions for proper centering
    ViewportSize viewport = Viewport_Get();
    
    Camera2D cam = {0};
    
    // Offset is the screen center - ensures camera target is always centered
    // regardless of window size or aspect ratio
    cam.offset = (Vector2){
        (float)(viewport.width  * 0.5f),
        (float)(viewport.height * 0.5f)
    };
    
    // Target is the world position the camera is looking at
    cam.target = s_camera.position;
    
    // Apply zoom and rotation from our state
    cam.zoom = s_camera.zoom;
    cam.rotation = s_camera.rotation;
    
    return cam;
}

Rectangle creCamera_GetViewBounds(void) {
    ViewportSize viewport = Viewport_Get();
    
    // Calculate the visible area in world space
    // Account for zoom: higher zoom = smaller visible area
    float viewWidth = viewport.width / s_camera.zoom;
    float viewHeight = viewport.height / s_camera.zoom;
    
    // The view bounds are centered on the camera position
    Rectangle bounds = {
        .x = s_camera.position.x - (viewWidth * 0.5f),
        .y = s_camera.position.y - (viewHeight * 0.5f),
        .width = viewWidth,
        .height = viewHeight
    };
    
    // Note: This doesn't account for rotation. If rotation is used,
    // the game should expand the bounds accordingly for culling safety.
    // A rotated rectangle's axis-aligned bounding box is larger.
    if (s_camera.rotation != 0.0f) {
        // Expand bounds to cover rotated view (simple approximation)
        // Uses the diagonal as worst-case for any rotation angle
        float visibleDiagonal = s_internal.baseDiagonal / s_camera.zoom;

        bounds.x = s_camera.position.x - (visibleDiagonal * 0.5f);
        bounds.y = s_camera.position.y - (visibleDiagonal * 0.5f);
        bounds.width = visibleDiagonal;
        bounds.height = visibleDiagonal;
    }
    return bounds;
}

Rectangle creCamera_GetCullBounds(void) {
    Rectangle view = creCamera_GetViewBounds();

    return (Rectangle){
    .x = view.x - CAMERA_CULL_MARGIN,
    .y = view.y - CAMERA_CULL_MARGIN,
    .width = view.width + (CAMERA_CULL_MARGIN * 2.0f),
    .height = view.height + (CAMERA_CULL_MARGIN * 2.0f)
    };
}