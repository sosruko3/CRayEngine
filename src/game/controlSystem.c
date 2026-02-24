#include "controlSystem.h"
#include "raylib.h"
#include "entity_types.h"
#include "engine/ecs/cre_entityManager.h"
#include "engine/ecs/cre_entityRegistry.h"
#include "engine/platform/cre_input.h"
#include "engine/platform/cre_viewport.h"
#include "engine/systems/animation/cre_animationSystem.h"
#include "engine/systems/camera/cre_cameraSystem.h"
#include "engine/core/cre_config.h"
#include "atlas_data.h"
#include "engine/core/cre_commandBus.h"
#include "engine/systems/physics/cre_physicsSystem.h"
#include <assert.h>
#include <math.h>

#define SLEEP_RADIUS      2500.0f 
#define SLEEP_RADIUS_SQR  (SLEEP_RADIUS * SLEEP_RADIUS)
#define SPAWN_COUNT 500
#define PLAYER_SPEED 400.0f
#define SCALE_FACTOR 4.0f
#define ZOOM_RATE_PER_SEC 0.60f

static Entity s_cameraTarget = ENTITY_INVALID;

void ControlSystem_UpdateLogic(EntityRegistry* reg, float dt) {
    assert(reg && "reg is NULL");
    uint32_t maxBound = reg->max_used_bound;
    creRectangle cullBounds = cameraSystem_GetCullBounds();
    float boundMinX = cullBounds.x;
    float boundMaxX = cullBounds.x + cullBounds.width;
    float boundMinY = cullBounds.y;
    float boundMaxY = cullBounds.y + cullBounds.height;
    
    for (uint32_t i = 0; i < maxBound; i++) {
        if (!(reg->state_flags[i] & FLAG_ACTIVE)) continue;
        
        float posX = reg->pos_x[i];
        float posY = reg->pos_y[i];
        bool isOutofBounds = (posX < boundMinX || posX > boundMaxX ||
                      posY < boundMinY || posY > boundMaxY);

        switch (reg->types[i]) {
            case TYPE_PLAYER: {
                // Input Control
                float velX = 0, velY = 0;
                float speed = PLAYER_SPEED;
                if (Input_IsDown(ACTION_UP)) velY = -speed;
                if (Input_IsDown(ACTION_DOWN)) velY = speed;
                if (Input_IsDown(ACTION_LEFT)) velX = -speed;
                if (Input_IsDown(ACTION_RIGHT)) velX = speed;
                reg->vel_x[i] = velX;
                reg->vel_y[i] = velY;
                break;
            }
            case TYPE_PARTICLE: {
                if (isOutofBounds) {
                    Entity self = {.id = i, .generation = reg->generations[i]};
                    EntityManager_Destroy(reg, self);
                }                        
                break;
            }
            default: break;
        }
    }
}

void ControlSystem_ChangeZoom(float dt) {
    if (dt <= 0.0f) return;

    if (Input_IsDown(ACTION_PRIMARY)) {
        float zoomScale = expf(ZOOM_RATE_PER_SEC * dt);
        cameraSystem_SetZoom(cameraSystem_GetZoom() * zoomScale);
    }
    else if (Input_IsDown(ACTION_SECONDARY)) {
        float zoomScale = expf(-ZOOM_RATE_PER_SEC * dt);
        cameraSystem_SetZoom(cameraSystem_GetZoom() * zoomScale);
    }
}

void ControlSystem_SetCameraTarget(EntityRegistry* reg, Entity target) {
    s_cameraTarget = ENTITY_INVALID;

    if (reg && ENTITY_IS_VALID(target) && target.id < MAX_ENTITIES) {
        if (reg->generations[target.id] != target.generation) return;
        s_cameraTarget = target;
        cameraSystem_SetTargetEntity(target);
        cameraSystem_SetMode(CAM_MODE_FOLLOW);

        creVec2 pos = {reg->pos_x[target.id], reg->pos_y[target.id]};
        cameraSystem_SetPosition(pos);
    }
}

void ControlSystem_UpdateSleepState(EntityRegistry* reg) {
    assert(reg && "reg is NULL");
    if (!ENTITY_IS_VALID(s_cameraTarget)) return;
    if (s_cameraTarget.id >= MAX_ENTITIES) return;
    if (!(reg->state_flags[s_cameraTarget.id] & FLAG_ACTIVE)) return;
    if (reg->generations[s_cameraTarget.id] != s_cameraTarget.generation) return;
    
    float playerX = reg->pos_x[s_cameraTarget.id];
    float playerY = reg->pos_y[s_cameraTarget.id];
    uint32_t maxBound = reg->max_used_bound;

    for (uint32_t i = 0; i < maxBound; i++) {
        // Only check active, non-player entities
        if (!(reg->state_flags[i] & FLAG_ACTIVE)) continue;
        if (reg->types[i] == TYPE_PLAYER) continue;

        // Distance Check
        float dx = reg->pos_x[i] - playerX;
        float dy = reg->pos_y[i] - playerY;
        float distSqr = (dx * dx) + (dy * dy);

        // Set the Flag
        if (distSqr > SLEEP_RADIUS_SQR) {
            reg->state_flags[i] |= FLAG_CULLED; // Set bit
        } else {
            reg->state_flags[i] &= ~FLAG_CULLED; // Clear bit
        }
    }
}

void ControlSystem_HandleDebugSpawning(EntityRegistry* reg, CommandBus* bus) {
    assert(reg && "reg is NULL");
    assert(bus && "bus is NULL");

    if (IsKeyPressed(KEY_Z)) {
        ViewportSize v = Viewport_Get();
        for (int i = 0; i < SPAWN_COUNT; i++) {
            int x = GetRandomValue((int)(-8 * v.width),  (int)(v.width * 8));
            int y = GetRandomValue((int)(-8 * v.height), (int)(v.height * 8));

            uint64_t compMask = COMP_SPRITE | COMP_ANIMATION | COMP_PHYSICS | COMP_COLLISION_AABB;
            uint64_t flags = FLAG_ACTIVE | FLAG_VISIBLE | SET_LAYER(L_ENEMY) | SET_MASK(L_PLAYER | L_ENEMY);

            Entity e = EntityManager_Create(reg, TYPE_ENEMY, (creVec2){x, y}, compMask, flags);
            if (ENTITY_IS_VALID(e)) {
                reg->render_layer[e.id] = RENDER_LAYER_ENEMY;
                reg->batch_ids[e.id] = RENDER_BATCH_ENEMY;
                reg->vel_x[e.id] = (float)GetRandomValue(-20, 20);
                reg->vel_y[e.id] = (float)GetRandomValue(-20, 20);

                Command cmd = {
                    .type = CMD_PHYS_DEFINE,
                    .entity = (Entity){e.id, e.generation},
                    .physDef.material_id = MAT_DEFAULT,
                    .physDef.flags = 0,
                    .physDef.drag = 2.0f
                };
                CommandBus_Push(bus, cmd);
                //AnimationSystem_Play(reg, e.id, ANIM_CHARACTER_ZOMBIE_RUN, true);
            }
        }
    }

    if (IsKeyPressed(KEY_X)) {
        uint64_t compMask = COMP_SPRITE | COMP_PHYSICS | COMP_COLLISION_Circle;
        uint64_t flags = FLAG_ACTIVE | FLAG_VISIBLE | FLAG_ALWAYS_AWAKE |
                        SET_LAYER(L_ENEMY) | SET_MASK(L_PLAYER | L_BULLET | L_ENEMY);

        Entity e = EntityManager_Create(reg, TYPE_ENEMY, (creVec2){400, 400}, compMask, flags);
        if (ENTITY_IS_VALID(e)) {
            reg->render_layer[e.id] = RENDER_LAYER_ENEMY;
            reg->batch_ids[e.id] = RENDER_BATCH_ENEMY;
            reg->sprite_ids[e.id] = SPR_ENEMY_IDLE;
            reg->vel_x[e.id] = 20;
            reg->vel_y[e.id] = 20;

            Command cmd;
            cmd.type = CMD_PHYS_DEFINE;
            cmd.entity = (Entity){e.id, e.generation};
            cmd.physDef.material_id = MAT_DEFAULT;
            cmd.physDef.flags = 0;
            cmd.physDef.drag = 0.1f;
            CommandBus_Push(bus, cmd);
        }
    }
}

void ControlSystem_SpawnPlayer(EntityRegistry* reg, CommandBus* bus) {
    assert(reg && "reg is NULL");
    
    uint64_t compMask = COMP_SPRITE | COMP_ANIMATION | COMP_PHYSICS | COMP_COLLISION_AABB;
    uint64_t flags = FLAG_ACTIVE | FLAG_VISIBLE | FLAG_ALWAYS_AWAKE | SET_LAYER(L_PLAYER) | SET_MASK(L_ENEMY | L_BULLET);
    
    Entity player = EntityManager_Create(reg, TYPE_PLAYER, (creVec2){100, 200}, compMask, flags);
    if (ENTITY_IS_VALID(player)) {
        reg->render_layer[player.id] = RENDER_LAYER_PLAYER;
        reg->batch_ids[player.id] = RENDER_BATCH_PLAYER;
        reg->sprite_ids[player.id] = SPR_SOLDIER;
        
        Command cmd;
        cmd.type = CMD_PHYS_DEFINE;
        cmd.entity = (Entity){player.id,player.generation};
        cmd.physDef.material_id = MAT_PLAYER;
        cmd.physDef.flags = 0;
        cmd.physDef.drag = 0.1f;
        CommandBus_Push(bus,cmd);

        //AnimationSystem_Play(reg, player.id, ANIM_CHARACTER_ZOMBIE_RUN, true);
        ControlSystem_SetCameraTarget(reg, player);
    }
}

