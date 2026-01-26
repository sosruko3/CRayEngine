#include "animation.h"
#include <string.h> // for memset

void AnimationSystem_Init(AnimState* state) {
    memset(state,0,sizeof(AnimState));

    state->speedScale = ANIM_SPEED_ONE; // Default to 1.0x speed
    state->currentAnim = 0;             // Defaults to the first animation
}

void AnimationSystem_Play(AnimState* state, AnimID animID, bool forceReset) {
    // Check bounds using the generated ANIM_COUNT macro
    if (animID >= ANIM_COUNT) return;

    // if already playing this do nothing, unless forced
    if (state->currentAnim == animID && !forceReset && !state->finished) return;

    // Set new state
    state->currentAnim = animID;
    state->frameIndex = 0;
    state->timer = 0.0f;
    state->finished = 0;
    // Did not reset speedScale here for maybe flexibility, would fix that if there is a problem.
}

void AnimationSystem_Update(AnimState* state,float dt) {
    if (dt >0.05f) dt = 0.05f;

    // Get static data
    const AnimDef* def = &ASSET_ANIMS[state->currentAnim];

    // Early exit if non-looping animation is done
    // Logic: If finished AND (Normal Loop is false AND Override is not Force Loop)
    // Note: You could make loopOverride complex (0=UseDef, 1=ForceLoop, 2=ForceStop)
    // For now, let's assume the definition dictates looping unless we add logic.
    if (state->finished) return;

    /*  Calculate Effective Speed
        Formula: dt * (speedScale/ 4096.0f)
        Optimization: Multiple dt by speedScale first, then divite to minimize float ops?
    */
    float speedMultiplier = (float)state->speedScale / (float)ANIM_SPEED_ONE;
    float effectiveDt = dt * speedMultiplier; 

    state->timer += effectiveDt;

    // Advance Frames
    // While loop handles cases where low FPS causes multiple frame skips
    while (state->timer >= def->defaultSpeed) {
        state->timer -= def->defaultSpeed;

        state->frameIndex++;

        // Handle End of animation
        if (state->frameIndex >= def->frameCount) {
            bool shouldLoop = def->loop;

            // Apply Override logic if you add it later
            // if (state->loopOverride == 1) shouldLoop = true;

            if (shouldLoop) {
                state->frameIndex = 0;
            }
            else {
                state->frameIndex = def->frameCount -1; // Clamp to the last frame
                state->finished = 1;
                break;
            }
        }
    }
}   
void AnimationSystem_SetSpeed(AnimState* state,float multiplier) {
    // Convert float to fixed-point
    // e.g. 1.5f * 4096 = 6144
    state->speedScale = (uint16_t)(multiplier * ANIM_SPEED_ONE);
}

SpriteID AnimationSystem_GetCurrentSprite(const AnimState* state) {
    if (state->currentAnim >= ANIM_COUNT) return 0;

    const AnimDef* def =&ASSET_ANIMS[state->currentAnim];

    // The Python Script guarantees consecutive IDs:
    // StartID (10) + FrameIndex (2) = SpriteID (12)
    return (SpriteID)(def->startSpriteID + state->frameIndex);
}