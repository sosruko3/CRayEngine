#ifndef ANIMATION_H
#define ANIMATION_H

#include "config.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int startSpriteID;  // The ID of the first frame (e.g., 10)
    int frameCount;     // How many frames? (e.g., 4)
    float speed;        // Frames per second (e.g., 12.0f)
    bool loop;          // Should it repeat?
} AnimDef;


typedef struct {
    float timer;
    uint16_t currentAnimID;
    uint8_t finished;
    uint8_t flipX;
} AnimComponent;

void AnimationSystem_Update(float dt);
void AnimationSystem_Set(uint32_t entityID,uint16_t newAnimID);

extern AnimComponent comp_anim[MAX_ENTITIES];
extern const AnimDef ANIMATIONS[];
#endif