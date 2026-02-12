#include "controlSystem.h"
#include "raylib.h"
#include "entity_types.h"
#include "engine/ecs/cre_entityManager.h"
#include "engine/ecs/cre_entityRegistry.h"
#include "engine/platform/cre_input.h"
#include "engine/systems/animation/cre_animationSystem.h"
#include "engine/platform/cre_viewport.h"
#include "engine/systems/camera/cre_camera.h"
#include "engine/systems/camera/cre_cameraUtils.h"
#include "engine/core/cre_config.h"
#include "atlas_data.h"
#include "engine/core/cre_commandBus.h"
#include "engine/systems/physics/cre_physicsSystem.h"
#include <assert.h>

#define SLEEP_RADIUS      2500.0f 
#define SLEEP_RADIUS_SQR  (SLEEP_RADIUS * SLEEP_RADIUS)
#define PLAYER_SPEED 400.0f
#define SCALE_FACTOR 4.0f

static uint32_t s_cameraTargetID = MAX_ENTITIES + 1;

void ControlSystem_UpdateLogic(EntityRegistry* reg, float dt) {
    assert(reg && "reg is NULL");
    uint32_t maxBound = reg->max_used_bound;
    ViewportSize v = Viewport_Get();
    float boundMinX = -(v.width * 1.5f);
    float boundMaxX =  (v.width * 1.5f);
    float boundMinY = -(v.height * 1.5f);
    float boundMaxY =  (v.height * 1.5f);
    
    for (uint32_t i = 0; i < maxBound; i++) {
        if (!(reg->state_flags[i] & FLAG_ACTIVE)) continue;
        
        float posX = reg->pos_x[i];
        float posY = reg->pos_y[i];
        bool isOutofBounds = (posX < -(boundMinX) || posX > (boundMaxX) ||
                              posY < -(boundMinY) || posY > (boundMaxY));

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

void ControlSystem_ChangeZoom(void) {
    if (Input_IsDown(ACTION_PRIMARY)) {
        creCamera_SetZoom(creCamera_GetZoom() * 1.01f);
    }
    else if (Input_IsDown(ACTION_SECONDARY)) {
        creCamera_SetZoom(creCamera_GetZoom() * 0.99f);
    }
}

void ControlSystem_SetCameraTarget(EntityRegistry* reg, uint32_t entityID) {
    s_cameraTargetID = entityID;

    if (reg && s_cameraTargetID < MAX_ENTITIES) {
        creVec2 pos = {reg->pos_x[s_cameraTargetID], reg->pos_y[s_cameraTargetID]};
        creCamera_CenterOn(pos);
    }
}

void ControlSystem_UpdateCamera(EntityRegistry* reg) {
    assert(reg && "reg is NULL");
    if (s_cameraTargetID >= MAX_ENTITIES) return;
    
    if (reg->state_flags[s_cameraTargetID] & FLAG_ACTIVE) {
        creVec2 pos = {reg->pos_x[s_cameraTargetID], reg->pos_y[s_cameraTargetID]};
        creCamera_CenterOn(pos);
    }
}

void ControlSystem_UpdateSleepState(EntityRegistry* reg) {
    assert(reg && "reg is NULL");
    if (s_cameraTargetID >= MAX_ENTITIES) return;
    
    float playerX = reg->pos_x[s_cameraTargetID];
    float playerY = reg->pos_y[s_cameraTargetID];
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

void ControlSystem_SpawnPlayer(EntityRegistry* reg, CommandBus* bus) {
    assert(reg && "reg is NULL");
    
    uint64_t compMask = COMP_SPRITE | COMP_ANIMATION | COMP_PHYSICS | COMP_COLLISION_Circle;
    uint64_t flags = FLAG_ACTIVE | FLAG_VISIBLE | FLAG_ANIMATED | FLAG_ALWAYS_AWAKE | SET_LAYER(L_PLAYER) | SET_MASK(L_ENEMY | L_BULLET);
    
    Entity player = EntityManager_Create(reg, TYPE_PLAYER, (creVec2){100, 200}, compMask, flags);
    if (ENTITY_IS_VALID(player)) {
        reg->sprite_ids[player.id] = SPR_PLAYER_IDLE0;
        
        Command cmd;
        cmd.type = CMD_PHYS_DEFINE;
        cmd.entity = (Entity){player.id,player.generation};
        cmd.physDef.material_id = MAT_PLAYER;
        cmd.physDef.flags = 0;
        cmd.physDef.drag = 0.1f;
        CommandBus_Push(bus,cmd);

        AnimationSystem_Play(reg, player.id, ANIM_CHARACTER_ZOMBIE_RUN, true);
        ControlSystem_SetCameraTarget(reg, player.id);
    }
}

