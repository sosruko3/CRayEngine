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
#include "engine/core/viewport.h"
#include "engine/physics/spatial_hash.h"
#include "engine/core/cre_camera.h"
#include "engine/core/cre_camera_utils.h"

#define SLEEP_RADIUS      3000.0f 
#define SLEEP_RADIUS_SQR  (SLEEP_RADIUS * SLEEP_RADIUS)
#define PLAYER_SPEED 500.0f
#define PARTICLE_COUNT 1000
#define SCALE_FACTOR 4.0f
static uint32_t s_cameraTargetID = MAX_ENTITIES + 1;

// DO NOT FORGET REMOVING GetScreenWidth()s later on.

void System_UpdateLogic(float dt) {
    for (uint32_t i = 0; i < MAX_ENTITIES; i++) {
        // not using get func here is important!
        EntityData* e = &entityStore[i];
        if (!(e->flags & FLAG_ACTIVE)) continue;
        ViewportSize v = Viewport_Get();
        bool isOutofBounds = (e->position.x < -100 || e->position.x > v.width + 100 ||
                            e->position.y < -100 || e->position.y > v.height + 100);

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
    if (IsKeyDown(KEY_Z)) {
        for (int i = 0; i < PARTICLE_COUNT; i++) {
            // 1. Random Position
            int x = GetRandomValue((-2)*GetScreenWidth(), GetScreenWidth()*2); // FOR DEBUG
            int y = GetRandomValue((-2)*GetScreenHeight(), GetScreenHeight()*2); // Change GetScreenWidth,height to viewport's things later
            
            Entity e = EntityManager_Create(TYPE_ENEMY, (Vector2){x, y});
            EntityData* data = EntityManager_Get(e);
            if (data) {
                data->velocity.x = GetRandomValue(-200, 200); // Fast random movement
                data->velocity.y = GetRandomValue(-200, 200);
                data->color = WHITE;
                data->size = (Vector2) {32*SCALE_FACTOR,32*SCALE_FACTOR};
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
    Rectangle cullRect = creCamera_GetCullBounds();
    static uint32_t visibleEntities[MAX_VISIBLE_ENTITIES];
    int visibleCount = SpatialHash_Query(
        (int)cullRect.x, 
        (int)cullRect.y, 
        (int)cullRect.width, 
        (int)cullRect.height, 
        visibleEntities, 
        MAX_VISIBLE_ENTITIES
    );
    for (uint32_t i = 0; i < visibleCount; i++) {
        uint32_t id = visibleEntities[i];
        EntityData* e = &entityStore[id];

        // Double checking
        if ((e->flags & (FLAG_ACTIVE | FLAG_VISIBLE)) == (FLAG_ACTIVE | FLAG_VISIBLE)) {
            creRenderer_DrawSprite(
                e->spriteID,
                e->position,
                e->size,
                (Vector2){0.5f,0.5f}, // Central Pivot
                e->rotation,
                comp_anim[id].flipX,
                comp_anim[id].flipY,
                e->color
            );
        }
    }
}
void SystemTestSpawn(void) {
    if (IsKeyPressed(KEY_X)) {
        Entity e = EntityManager_Create(TYPE_ENEMY,(Vector2) {400,400});
        EntityData* e_data = EntityManager_Get(e);
        if (e_data) {
            e_data->spriteID   = SPR_enemy_idle; // Default sprite
            e_data->velocity   = (Vector2){20,20};
            e_data->color      = RAYWHITE;
            e_data->size       = (Vector2){16*SCALE_FACTOR,16*SCALE_FACTOR};
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
        pData->size = (Vector2){16*SCALE_FACTOR,16*SCALE_FACTOR};
        pData->velocity = (Vector2) {150,150};
        pData->flags |= FLAG_ACTIVE | FLAG_VISIBLE | FLAG_BOUNCY | FLAG_ANIMATED;
        pData->flags |= SET_LAYER(L_ENEMY);
        pData->flags |= SET_MASK(L_ENEMY | L_BULLET);
        AnimationSystem_Set(player.id,ANIM_PLAYER_IDLE);
        System_SetCameraTarget(player.id); // Hardcoded camera to follow player. !!!!!
    }
}

void System_SetCameraTarget(uint32_t entityID) {
    s_cameraTargetID = entityID;

    if (s_cameraTargetID < MAX_ENTITIES) {
        creCamera_CenterOn(entityStore[s_cameraTargetID].position);
    }
}

void System_UpdateCamera(void) {
    if (s_cameraTargetID >= MAX_ENTITIES) return;

    EntityData* target = &entityStore[s_cameraTargetID];

    if (target->flags & FLAG_ACTIVE) {
        creCamera_CenterOn(target->position);
    }
}

void System_UpdateSleepState(void) {
    // 1. Get Player Position (The Anchor)
    if (s_cameraTargetID >= MAX_ENTITIES) return;
    Vector2 playerPos = entityStore[s_cameraTargetID].position;

    for (uint32_t i = 0; i < MAX_ENTITIES; i++) {
        EntityData* e = &entityStore[i];
        
        // Only check active, non-player entities
        if (!(e->flags & FLAG_ACTIVE) || (e->type == TYPE_PLAYER)) continue;

        // 2. Distance Check
        float dx = e->position.x - playerPos.x;
        float dy = e->position.y - playerPos.y;
        float distSqr = (dx*dx) + (dy*dy);

        // 3. Set the Flag
        if (distSqr > SLEEP_RADIUS_SQR) {
            e->flags |= FLAG_CULLED; // Set bit
        } else {
            e->flags &= ~FLAG_CULLED; // Clear bit
        }
    }
}
void System_ChangeZoom(void) {
    if (Input_IsDown(ACTION_PRIMARY)) {
        creCamera_SetZoom(creCamera_GetZoom()*1.01);
    }
    else if (Input_IsDown(ACTION_SECONDARY)) {
        creCamera_SetZoom(creCamera_GetZoom()* 0.99);
    }
}
// For next update
//void System_DrawDebug(void) {
//    if (IsKeyDown(KEY_F1)) {
//        // 1. Get Camera Center (The "Eye" of the player)
//        // We use this to draw the reference rings
//        Camera2D cam = creCamera_GetInternal();
//        Vector2 center = cam.target; 
//        
//        // 2. Draw the "Logic Zones"
//        // Red Ring = The Wall (Where entities stop thinking)
//        // Match this to your SLEEP_RADIUS/WAKE_DISTANCE (e.g., 3000)
//        DrawCircleLines((int)center.x, (int)center.y, 3000, RED);
//        
//        // Green Box = The Screen (Where entities get drawn)
//        // 1920x1080 centered on player
//        DrawRectangleLines(
//            (int)(center.x - 960), 
//            (int)(center.y - 540), 
//            1920, 1080, GREEN
//        );
//        
//        // 3. Draw the "God View" of 40k Entities
//        for (uint32_t i = 0; i < MAX_ENTITIES; i++) {
//            EntityData* e = &entityStore[i];
//            
//            // Skip dead/empty slots
//            if (!(e->flags & FLAG_ACTIVE)) continue;
//            
//            Color debugColor = WHITE;
//            
//            // HIERARCHY OF STATES
//            if (e->flags & FLAG_CULLED) {
//                // RED: "I am too far away. CPU is ignoring me."
//                debugColor = RED; 
//            } 
//            else if (e->flags & FLAG_SLEEPING) {
//                // YELLOW: "I am close, but standing still. Physics is skipping me."
//                debugColor = YELLOW;
//            } 
//            else {
//                // GREEN: "I am active and moving. CPU is calculating my physics."
//                debugColor = GREEN;
//            }
//            
//            // Draw a generic "pixel" for the entity
//            DrawPixel((int)e->position.x, (int)e->position.y, debugColor);
//        }
//        
//        DrawText("DEBUG MODE: Red=Culled, Yellow=Sleep, Green=Active", 
//            (int)(center.x - 900), (int)(center.y - 500), 20, WHITE);
//    }
//}