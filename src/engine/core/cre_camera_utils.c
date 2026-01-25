#include "cre_camera_utils.h"
#include "cre_camera.h"
#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>

// ============================================================================
// Smart Utility Functions - The Toolbox
// ============================================================================

// Will refactor these later on,these are temporary.
void creCamera_LerpTo(Vector2 target, float speed, float dt) {
    Vector2 currentPos = creCamera_GetPosition();
    
    // Exponential decay interpolation for smooth camera following
    // Formula: new_pos = current + (target - current) * (1 - e^(-speed * dt))
    // Simplified to: new_pos = Lerp(current, target, 1 - e^(-speed * dt))
    float t = 1.0f - expf(-speed * dt);
    
    Vector2 newPos = {
        .x = currentPos.x + (target.x - currentPos.x) * t,
        .y = currentPos.y + (target.y - currentPos.y) * t
    };
    
    creCamera_SetPosition(newPos);
}

void creCamera_ApplyShake(float intensity) {
    if (intensity <= 0.0f) return;
    
    Vector2 currentPos = creCamera_GetPosition();
    
    // Generate random offset within intensity bounds
    // Using simple random for game-quality shake
    float offsetX = ((float)rand() / (float)RAND_MAX * 2.0f - 1.0f) * intensity;
    float offsetY = ((float)rand() / (float)RAND_MAX * 2.0f - 1.0f) * intensity;
    
    Vector2 shakenPos = {
        .x = currentPos.x + offsetX,
        .y = currentPos.y + offsetY
    };
    
    creCamera_SetPosition(shakenPos);
}

Vector2 creCamera_ScreenToWorld(Vector2 screenPos,ViewportSize vp) {
    Camera2D cam = creCamera_GetInternal(vp);
    return GetScreenToWorld2D(screenPos, cam);
}

Vector2 creCamera_WorldToScreen(Vector2 worldPos,ViewportSize vp) {
    Camera2D cam = creCamera_GetInternal(vp);
    return GetWorldToScreen2D(worldPos, cam);
}

void creCamera_CenterOn(Vector2 targetPosition) {
    creCamera_SetPosition(targetPosition);
}