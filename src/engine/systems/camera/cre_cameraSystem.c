#include "cre_cameraSystem.h"
#include "cre_cameraUtils.h"
#include "engine/core/cre_commandBus.h"
#include "engine/core/cre_config.h"
#include "engine/ecs/cre_entityRegistry.h"
#include <assert.h>
#include <math.h>

typedef struct CameraSystemState {
    creVec2 position;
    float zoom;
    float rotation;

    Entity targetEntity;
    CameraSystemMode mode;
    float smoothSpeed;

    float shakeTimer;
    float shakeIntensity;
    creVec2 shakeOffset;

    ViewportSize cachedVp;
    float baseDiagonal;
} CameraSystemState;

static CameraSystemState s_cameraSystem = {
    .position = {0.0f, 0.0f},
    .zoom = 1.0f,
    .rotation = 0.0f,
    .targetEntity = ENTITY_INVALID,
    .mode = CAM_MODE_MANUAL,
    .smoothSpeed = 10.0f,
    .shakeTimer = 0.0f,
    .shakeIntensity = 0.0f,
    .shakeOffset = {0.0f, 0.0f},
    .cachedVp = {0.0f, 0.0f, 0.0f},
    .baseDiagonal = 0.0f
};

static bool cameraSystem_IsTargetValid(const EntityRegistry* reg, Entity target) {
    if (!ENTITY_IS_VALID(target)) return false;
    if (target.id >= MAX_ENTITIES) return false;
    if (reg->generations[target.id] != target.generation) return false;
    if (!(reg->state_flags[target.id] & FLAG_ACTIVE)) return false;
    return true;
}

void cameraSystem_UpdateViewportCache(ViewportSize vp) {
    s_cameraSystem.cachedVp = vp;
    s_cameraSystem.baseDiagonal = sqrtf((vp.width * vp.width) + (vp.height * vp.height));
}

void cameraSystem_Init(ViewportSize vp) {
    s_cameraSystem.position = (creVec2){0.0f, 0.0f};
    s_cameraSystem.zoom = 1.0f;
    s_cameraSystem.rotation = 0.0f;
    s_cameraSystem.targetEntity = ENTITY_INVALID;
    s_cameraSystem.mode = CAM_MODE_MANUAL;
    s_cameraSystem.smoothSpeed = 10.0f;
    s_cameraSystem.shakeTimer = 0.0f;
    s_cameraSystem.shakeIntensity = 0.0f;
    s_cameraSystem.shakeOffset = (creVec2){0.0f, 0.0f};
    cameraSystem_UpdateViewportCache(vp);
}

void cameraSystem_ProcessCommands(EntityRegistry* reg, const CommandBus* bus) {
    assert(reg && "reg is NULL");
    assert(bus && "bus is NULL");

    CommandIterator iter = CommandBus_GetIterator(bus);
    const Command* cmd;

    while (CommandBus_Next(bus, &iter, &cmd)) {
        switch (cmd->type) {
            default:
                break;
        }
    }
}

void cameraSystem_Update(EntityRegistry* reg, const CommandBus* bus, float dt) {
    assert(reg && "reg is NULL");
    assert(bus && "bus is NULL");

    if (dt > 0.05f) dt = 0.05f;
    if (dt < 0.0f) dt = 0.0f;

    cameraSystem_ProcessCommands(reg, bus);

    if (s_cameraSystem.mode == CAM_MODE_FOLLOW &&
        cameraSystem_IsTargetValid(reg, s_cameraSystem.targetEntity)) {
        const uint32_t id = s_cameraSystem.targetEntity.id;
        const creVec2 targetPos = { reg->pos_x[id], reg->pos_y[id] };

        if (s_cameraSystem.smoothSpeed > 0.0f) {
            s_cameraSystem.position = cameraUtils_Lerp(
                s_cameraSystem.position,
                targetPos,
                s_cameraSystem.smoothSpeed,
                dt
            );
        } else {
            s_cameraSystem.position = targetPos;
        }
    }

    if (s_cameraSystem.shakeTimer > 0.0f && s_cameraSystem.shakeIntensity > 0.0f) {
        s_cameraSystem.shakeOffset = cameraUtils_RandomShakeOffset(s_cameraSystem.shakeIntensity);
        s_cameraSystem.shakeTimer -= dt;
        if (s_cameraSystem.shakeTimer <= 0.0f) {
            s_cameraSystem.shakeTimer = 0.0f;
            s_cameraSystem.shakeIntensity = 0.0f;
            s_cameraSystem.shakeOffset = (creVec2){0.0f, 0.0f};
        }
    } else {
        s_cameraSystem.shakeOffset = (creVec2){0.0f, 0.0f};
    }
}

void cameraSystem_SetPosition(creVec2 position) {
    s_cameraSystem.position = position;
}

creVec2 cameraSystem_GetPosition(void) {
    return s_cameraSystem.position;
}

void cameraSystem_SetZoom(float zoom) {
    if (zoom < MIN_ZOOM) zoom = MIN_ZOOM;
    if (zoom > MAX_ZOOM) zoom = MAX_ZOOM;
    s_cameraSystem.zoom = zoom;
}

float cameraSystem_GetZoom(void) {
    return s_cameraSystem.zoom;
}

void cameraSystem_SetRotation(float rotation) {
    s_cameraSystem.rotation = rotation;
}

float cameraSystem_GetRotation(void) {
    return s_cameraSystem.rotation;
}

void cameraSystem_SetMode(CameraSystemMode mode) {
    s_cameraSystem.mode = mode;
}

CameraSystemMode cameraSystem_GetMode(void) {
    return s_cameraSystem.mode;
}

void cameraSystem_SetTargetEntity(Entity target) {
    s_cameraSystem.targetEntity = target;
}

Entity cameraSystem_GetTargetEntity(void) {
    return s_cameraSystem.targetEntity;
}

void cameraSystem_SetSmoothSpeed(float smoothSpeed) {
    if (smoothSpeed < 0.0f) smoothSpeed = 0.0f;
    s_cameraSystem.smoothSpeed = smoothSpeed;
}

float cameraSystem_GetSmoothSpeed(void) {
    return s_cameraSystem.smoothSpeed;
}

void cameraSystem_StartShake(float duration, float intensity) {
    if (duration <= 0.0f || intensity <= 0.0f) return;
    s_cameraSystem.shakeTimer = duration;
    s_cameraSystem.shakeIntensity = intensity;
}

Camera2D cameraSystem_GetInternal(void) {
    const ViewportSize vp = s_cameraSystem.cachedVp;

    Camera2D cam = {0};
    cam.offset = (Vector2){ vp.width * 0.5f, vp.height * 0.5f };
    cam.target = (Vector2){
        s_cameraSystem.position.x + s_cameraSystem.shakeOffset.x,
        s_cameraSystem.position.y + s_cameraSystem.shakeOffset.y
    };
    cam.zoom = s_cameraSystem.zoom;
    cam.rotation = s_cameraSystem.rotation;
    return cam;
}

creRectangle cameraSystem_GetViewBounds(void) {
    const ViewportSize vp = s_cameraSystem.cachedVp;

    float viewWidth = vp.width / s_cameraSystem.zoom;
    float viewHeight = vp.height / s_cameraSystem.zoom;

    creRectangle bounds = {
        .x = s_cameraSystem.position.x - (viewWidth * 0.5f),
        .y = s_cameraSystem.position.y - (viewHeight * 0.5f),
        .width = viewWidth,
        .height = viewHeight
    };

    if (s_cameraSystem.rotation != 0.0f) {
        float visibleDiagonal = s_cameraSystem.baseDiagonal / s_cameraSystem.zoom;
        bounds.x = s_cameraSystem.position.x - (visibleDiagonal * 0.5f);
        bounds.y = s_cameraSystem.position.y - (visibleDiagonal * 0.5f);
        bounds.width = visibleDiagonal;
        bounds.height = visibleDiagonal;
    }

    return bounds;
}

creRectangle cameraSystem_GetCullBounds(void) {
    creRectangle view = cameraSystem_GetViewBounds();

    return (creRectangle){
        .x = view.x - CAMERA_CULL_MARGIN,
        .y = view.y - CAMERA_CULL_MARGIN,
        .width = view.width + (CAMERA_CULL_MARGIN * 2.0f),
        .height = view.height + (CAMERA_CULL_MARGIN * 2.0f)
    };
}

const CameraSystemState* cameraSystem_GetState(void) {
    return &s_cameraSystem;
}