#include "cre_cameraUtils.h"
#include "cre_camera.h"
#include "raylib.h"
#include "engine/platform/cre_viewport.h"
#include "raymath.h"
#include <stdlib.h>
#include "engine/core/cre_typesMacro.h"


// ============================================================================
// Smart Utility Functions - The Toolbox
// ============================================================================

// Will refactor these later on,these are temporary.
void creCamera_LerpTo(creVec2 target, float speed, float dt) {
    creVec2 currentPos = creCamera_GetPosition();
    
    // Exponential decay interpolation for smooth camera following
    // Formula: new_pos = current + (target - current) * (1 - e^(-speed * dt))
    // Simplified to: new_pos = Lerp(current, target, 1 - e^(-speed * dt))
    float t = 1.0f - expf(-speed * dt);
    
    creVec2 newPos = {
        .x = currentPos.x + (target.x - currentPos.x) * t,
        .y = currentPos.y + (target.y - currentPos.y) * t
    };
    
    creCamera_SetPosition(newPos);
}

void creCamera_ApplyShake(float intensity) {
    if (intensity <= 0.0f) return;
    
    creVec2 currentPos = creCamera_GetPosition();
    
    // Generate random offset within intensity bounds
    // Using simple random for game-quality shake
    float offsetX = ((float)rand() / (float)RAND_MAX * 2.0f - 1.0f) * intensity;
    float offsetY = ((float)rand() / (float)RAND_MAX * 2.0f - 1.0f) * intensity;
    
    creVec2 shakenPos = {
        .x = currentPos.x + offsetX,
        .y = currentPos.y + offsetY
    };
    
    creCamera_SetPosition(shakenPos);
}

creVec2 creCamera_ScreenToWorld(creVec2 screenPos,ViewportSize vp) {
    Camera2D cam = creCamera_GetInternal(vp);
    Vector2 result = GetScreenToWorld2D(R_VEC(screenPos), cam);
    return (creVec2){result.x, result.y};
}

creVec2 creCamera_WorldToScreen(creVec2 worldPos,ViewportSize vp) {
    Camera2D cam = creCamera_GetInternal(vp);
    Vector2 result = GetWorldToScreen2D(R_VEC(worldPos), cam);
    return (creVec2){result.x, result.y};
}

void creCamera_CenterOn(creVec2 targetPosition) {
    creCamera_SetPosition(targetPosition);
}