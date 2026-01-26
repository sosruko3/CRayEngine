#ifndef ANIMATION_H
#define ANIMATION_H

#include "config.h"
#include <stdint.h>
#include <stdbool.h>
#include "atlas_data.h"
#include <assert.h>

// Constants for Fixed-point Math
#define ANIM_SPEED_ONE 4096     // 1.0x speed
#define ANIM_SPEED_HALF 2048    // 0.5x speed
#define ANIM_SPEED_DOUBLE 8192  // 2.0x speed

typedef struct {
    float timer;         // Time accumulator for the next frame.      4 bytes
    uint16_t currentAnim;  // Which animation is playing?             2 bytes
    uint16_t frameIndex; // Current Frame (0 to frameCount -1)        2 bytes
    uint8_t finished;       // Did the non-looping animation end?     1 bytes
    uint8_t loopOverride;   // If you want to force a loop.           1 bytes
    uint16_t speedScale;     // Can store 0.0-2.x speed multiplier.   2 bytes
    uint8_t padding[4];     // Padding                                4 bytes                                             
} AnimState;

// This line is your "Insurance Policy" - the build will fail if the size is wrong.
static_assert(sizeof(AnimState) == 16, "AnimState must be exactly 16 bytes for cache efficiency");

void AnimationSystem_Init(AnimState* state);
void AnimationSystem_Play(AnimState* state, AnimID animID, bool forceReset);
void AnimationSystem_Update(AnimState* state,float dt);
void AnimationSystem_SetSpeed(AnimState* state, float multiplier);
SpriteID AnimationSystem_GetCurrentSprite(const AnimState* state);

#endif