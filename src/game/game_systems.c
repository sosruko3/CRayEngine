#include "game_systems.h"
#include "entity_types.h"
#include "engine/core/entity_manager.h"
#include "engine/core/logger.h"
#include "raylib.h"
#include "../game_config.h"
#include "engine/core/input.h"
#include "engine/core/config.h"
#include "engine/core/cre_renderer.h"
#include  "engine/core/entity.h" // for layer/mask macros
#include "engine/core/asset_manager.h"
#include "atlas_data.h"
#include "game_animation.h"


#define PLAYER_SPEED 200.0f
#define PARTICLE_COUNT 1000
#define SCALE_FACTOR 4.0f

// DO NOT FORGET REMOVING GetScreenWidth()s later on.

void System_UpdateLogic(float dt) {
    for (uint32_t i = 0; i < MAX_ENTITIES; i++) {
        // not using get func here is important!
        EntityData* e = &entityStore[i];
        if (!(e->flags & FLAG_ACTIVE)) continue;
        bool isOutofBounds = (e->position.x < -100 || e->position.x > GetScreenWidth() + 100 ||
        e->position.y < -100 || e->position.y > GetScreenHeight() + 100);

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
            case TYPE_PARTICLE: {
                // Bounce Logic
                //if (e->position.x < 0 || e->position.x > SCREEN_WIDTH) e->velocity.x *= -1;
                //if (e->position.y < 0 || e->position.y > SCREEN_HEIGHT) e->velocity.y *= -1;
                if (isOutofBounds) {
                    Entity self = {.id = i, .generation = e->generation};
                        EntityManager_Destroy(self);
                    }                        
                break;
            }
            default: break;
        }
    }
}
void System_HandleDebugInput(void) {
    if (IsKeyPressed(KEY_Z)) {
        for (int i = 0; i < PARTICLE_COUNT; i++) {
            // 1. Random Position
            int x = GetRandomValue(0, GetScreenWidth()*4); // FOR DEBUG
            int y = GetRandomValue(0, GetScreenHeight()*4);
            
            Entity e = EntityManager_Create(TYPE_ENEMY, (Vector2){x, y});
            EntityData* data = EntityManager_Get(e);
            if (data) {
                data->velocity.x = GetRandomValue(-200, 200); // Fast random movement
                data->velocity.y = GetRandomValue(-200, 200);
                data->color = WHITE;
                data->size = (Vector2) {16*SCALE_FACTOR,16*SCALE_FACTOR};
                data->flags |= FLAG_ACTIVE | FLAG_VISIBLE;
                data->flags |= SET_LAYER(L_ENEMY);
                data->flags |= SET_MASK(L_PLAYER | L_ENEMY);
                //data->spriteID = SPR_cactus;
                AnimationSystem_Set(e.id,ANIM_ZOMBIE_RUN);
            }
        }
        Log(LOG_LVL_INFO,"Spawned %d entities!\n",PARTICLE_COUNT);
    }
}
void System_DrawEntities(void) {
    for (uint32_t i = 0; i < MAX_ENTITIES; i++) {
        EntityData* e = &entityStore[i];
        if ((entityStore[i].flags & FLAG_ACTIVE) && (entityStore[i].flags & FLAG_VISIBLE)) {
            creRenderer_DrawSprite(
                e->spriteID,
                e->position,
                (Vector2){0.5f,0.5f}, // Central Pivot
                e->rotation,
                1.0f, // Letting 1080p canvas handle it
                comp_anim[i].flipX,
                comp_anim[i].flipY,
                e->color
            );
        }
    }
}
void SystemTestSpawn(void) {
    if (Input_IsPressed(ACTION_SECONDARY)) {
        Entity e = EntityManager_Create(TYPE_ENEMY,(Vector2) {400,400});
        EntityData* e_data = EntityManager_Get(e);
        if (e_data) {
            e_data->spriteID   = SPR_enemy_idle; // Default sprite
            e_data->velocity   = (Vector2){20,20};
            e_data->color      = RAYWHITE;
            e_data->size       = (Vector2){24*SCALE_FACTOR,24*SCALE_FACTOR};
            e_data-> flags    |= FLAG_ACTIVE | FLAG_VISIBLE | FLAG_SOLID; // Active and visible
            e_data-> flags    |= SET_LAYER(L_ENEMY); //  = I am an enemy
            e_data-> flags    |= SET_MASK(L_PLAYER | L_BULLET | L_ENEMY); // = Hit players and bullets
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

void SpawnPlayer(void) {
    Entity player = EntityManager_Create(TYPE_PLAYER,(Vector2){100,200});
    EntityData* pData = EntityManager_Get(player);
    if (pData) {
        pData->spriteID = SPR_player_idle0;
        pData->size = (Vector2){24*SCALE_FACTOR,24*SCALE_FACTOR};
        pData->velocity = (Vector2) {150,150};
        pData->flags |= FLAG_ACTIVE | FLAG_VISIBLE | FLAG_BOUNCY | FLAG_ANIMATED;
        pData->flags |= SET_LAYER(L_ENEMY);
        pData->flags |= SET_MASK(L_ENEMY | L_BULLET);
        AnimationSystem_Set(player.id,ANIM_PLAYER_IDLE);
    }
}