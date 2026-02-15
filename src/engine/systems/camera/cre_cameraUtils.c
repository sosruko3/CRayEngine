#include "cre_cameraUtils.h"
#include "raylib.h"
#include <stdlib.h>
#include <math.h>
#include "engine/core/cre_typesMacro.h"


// ============================================================================
// Smart Utility Functions - The Toolbox
// ============================================================================

creVec2 cameraUtils_Lerp(creVec2 current, creVec2 target, float speed, float dt) {
    float t = 1.0f - expf(-speed * dt);

    return (creVec2){
        .x = current.x + (target.x - current.x) * t,
        .y = current.y + (target.y - current.y) * t
    };
}

creVec2 cameraUtils_RandomShakeOffset(float intensity) {
    if (intensity <= 0.0f) return (creVec2){0.0f, 0.0f};

    float offsetX = ((float)rand() / (float)RAND_MAX * 2.0f - 1.0f) * intensity;
    float offsetY = ((float)rand() / (float)RAND_MAX * 2.0f - 1.0f) * intensity;

    return (creVec2){offsetX, offsetY};
}

creVec2 cameraUtils_ScreenToWorld(creVec2 screenPos, Camera2D cam) {
    Vector2 result = GetScreenToWorld2D(R_VEC(screenPos), cam);
    return (creVec2){result.x, result.y};
}

creVec2 cameraUtils_WorldToScreen(creVec2 worldPos, Camera2D cam) {
    Vector2 result = GetWorldToScreen2D(R_VEC(worldPos), cam);
    return (creVec2){result.x, result.y};
}