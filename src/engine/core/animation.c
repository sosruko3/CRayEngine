#include "animation.h"
#include "game/atlas/atlas_data.h"
#include "entity_manager.h"

AnimComponent comp_anim[MAX_ENTITIES];

void AnimationSystem_Update(float dt) {
    if (dt >0.05f) dt = 0.05f;

    for (int i = 0;i < MAX_ENTITIES;i++) {
        EntityData*e = &entityStore[i];

        if(!(e->flags & (FLAG_ACTIVE | FLAG_ANIMATED))) continue;
        
        AnimComponent* state = &comp_anim[i];
        // Horizontal Flipping (Velocity based)
        if (e->velocity.x > 0.1f) state->flipX = 0; // Face Right
        else if (e->velocity.x < -0.1f) state->flipX = 1; // Face Left

        // Vertical flip
        // if (e->velocity.y > 0.1f)        state->flipY = 0;
        // else if (e-> velocity.y < -0.1f) state->flipY = true;

        const AnimDef* def = &ANIMATIONS[state->currentAnimID];
        state->timer += dt;

        int currentFrame = (int)(state->timer * def->speed);
        if (def->loop) currentFrame %= def->frameCount;
        else {
            if (currentFrame >= def->frameCount) {
                currentFrame = def->frameCount -1;
                state->finished = 1;
            }
        }
        e->spriteID = (uint16_t)(def->startSpriteID+currentFrame);
    }
}
void AnimationSystem_Set(uint32_t entityID, uint16_t newAnim) {
    // 1. Only reset if it's actually a different animation
    if (comp_anim[entityID].currentAnimID != newAnim) {
        comp_anim[entityID].currentAnimID = newAnim;
        comp_anim[entityID].timer = 0.0f;
        comp_anim[entityID].finished = 0;
    }
}

