#include "game_systems.h"
#include "entity_types.h"
#include "engine/core/entity_manager.h"
#include "engine/core/logger.h"
#include "raylib.h"
#include "../game_config.h"
#include "engine/core/input.h"
#include "engine/core/config.h"

#define PLAYER_SPEED 200.0f
#define PARTICLE_RADIUS 3.0f
#define PARTICLE_COUNT 1000

void System_UpdateLogic(float dt) {
    for (uint32_t i = 0; i < MAX_ENTITIES; i++) {
        // not using get func here is important!
        EntityData* e = &entityStore[i];
        if (!e->isActive) continue; 

        switch (e->type) {
            case TYPE_PLAYER: {

                // Input Control
                e->velocity = (Vector2){0, 0}; // Stop if no key
                float speed = PLAYER_SPEED;
                if (Input_IsDown(ACTION_UP)) e->velocity.y = -speed;
                if (Input_IsDown(ACTION_DOWN)) e->velocity.y = speed;
                if (Input_IsDown(ACTION_LEFT)) e->velocity.x = -speed;
                if (Input_IsDown(ACTION_RIGHT)) e->velocity.x = speed;
                break;
            }

            case TYPE_PARTICLE:
                // Bounce Logic
                if (e->position.x < 0 || e->position.x > SCREEN_WIDTH) e->velocity.x *= -1;
                if (e->position.y < 0 || e->position.y > SCREEN_HEIGHT) e->velocity.y *= -1;
                break;
            
            default: break;
        }
    }
}

void System_HandleDebugInput(void) {
    if (IsKeyPressed(KEY_Z)) {
        for (int i = 0; i < PARTICLE_COUNT; i++) {
            // 1. Random Position
            int x = GetRandomValue(0, SCREEN_WIDTH);
            int y = GetRandomValue(0, SCREEN_HEIGHT);
            
            Entity e = EntityManager_Create(TYPE_PARTICLE, (Vector2){x, y});
            EntityData* data = EntityManager_Get(e);
            if (data) {
                data->velocity.x = GetRandomValue(-200, 200); // Fast random movement
                data->velocity.y = GetRandomValue(-200, 200);
                
                // Random Color
                data->color = (Color){ 
                    GetRandomValue(50, 255), 
                    GetRandomValue(50, 255), 
                    GetRandomValue(50, 255), 
                    255 
                };
                data->radius = PARTICLE_RADIUS; // Tiny particles
            }
        }
        Log(LOG_LVL_INFO,"Spawned %d entities!\n",PARTICLE_COUNT);
    }
}

void System_DrawEntities(void) {
    for (uint32_t i = 0; i < MAX_ENTITIES; i++) {
        if (entityStore[i].isActive) {
            DrawCircleV(entityStore[i].position, entityStore[i].radius, entityStore[i].color);
        }
    }
}