#include "game_systems.h"
#include "entity_types.h"
#include "engine/core/entity_manager.h"
#include "engine/core/logger.h"
#include "raylib.h"
#include "../game_config.h"
#include "engine/core/input.h"
#include "engine/core/config.h"
#include  "engine/core/entity.h" // for layer/mask macros

#define PLAYER_SPEED 200.0f
#define PARTICLE_COUNT 1000

void System_UpdateLogic(float dt) {
    for (uint32_t i = 0; i < MAX_ENTITIES; i++) {
        // not using get func here is important!
        EntityData* e = &entityStore[i];
        if (!(e->flags & FLAG_ACTIVE)) continue;
        bool isOutofBounds = (e->position.x < -100 || e->position.x > SCREEN_WIDTH + 100 ||
        e->position.y < -100 || e->position.y > SCREEN_HEIGHT + 100);

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
                //if (e->position.x < 0 || e->position.x > SCREEN_WIDTH) e->velocity.x *= -1;
                //if (e->position.y < 0 || e->position.y > SCREEN_HEIGHT) e->velocity.y *= -1;
                if (isOutofBounds) {
                    Entity self = {.id = i, .generation = e->generation};
                        EntityManager_Destroy(self);
                    }                        
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
                data->color = (Color){ 
                    GetRandomValue(50, 255), 
                    GetRandomValue(50, 255), 
                    GetRandomValue(50, 255), 
                    255 
                };
                data->size = (Vector2) {10,10};
                data->flags |= FLAG_ACTIVE | FLAG_VISIBLE;
                data->flags |= SET_LAYER(L_PARTICLE);
                data->flags |= SET_MASK(L_PLAYER);
            }
        }
        Log(LOG_LVL_INFO,"Spawned %d entities!\n",PARTICLE_COUNT);
    }
}
void System_DrawEntities(void) {
    for (uint32_t i = 0; i < MAX_ENTITIES; i++) {
        if (entityStore[i].flags & FLAG_ACTIVE) {
            DrawCircleV(entityStore[i].position, entityStore[i].size.x/2, entityStore[i].color);
        }
    }
}
void SystemTestSpawn(void) {
    if (Input_IsPressed(ACTION_SECONDARY)) {
        Entity e = EntityManager_Create(TYPE_ENEMY,(Vector2) {400,400});
        EntityData* e_data = EntityManager_Get(e);
        if (e_data) {
            e_data->velocity = (Vector2){20,20};
            e_data->color      = RAYWHITE;
            e_data->size       = (Vector2){40,40};
            e_data-> flags    |= FLAG_ACTIVE | FLAG_VISIBLE; // Active and visible
            e_data-> flags    |= SET_LAYER(L_ENEMY); //  = I am an enemy
            e_data-> flags    |= SET_MASK(L_PLAYER | L_BULLET); // = Hit players and bullets
        }
        Log(LOG_LVL_INFO,"Spawned Test Entity");
    }
}

int GetActiveEntityCount(void) {
    int count = 0;
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (entityStore[i].flags & FLAG_ACTIVE) {
            count++;
        }
    }
    return count;
}