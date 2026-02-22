#ifndef CRE_CAMERASYSTEM_H
#define CRE_CAMERASYSTEM_H

#include "engine/core/cre_types.h"
#include "engine/platform/cre_viewport.h"
#include "raylib.h"

typedef struct EntityRegistry EntityRegistry;
typedef struct CommandBus CommandBus;

typedef enum CameraSystemMode {
    CAM_MODE_MANUAL = 0,
    CAM_MODE_FOLLOW,
    CAM_MODE_CINEMATIC,
    CAM_MODE_LOCKED
} CameraSystemMode;

bool cameraSystem_IsTargetValid(const EntityRegistry* reg, Entity target);
void cameraSystem_Init(ViewportSize vp);
void cameraSystem_UpdateViewportCache(ViewportSize vp);

void cameraSystem_ProcessCommands(EntityRegistry* reg, CommandBus* bus);
void cameraSystem_Update(EntityRegistry* reg, CommandBus* bus, float dt);

void cameraSystem_SetPosition(creVec2 position);
creVec2 cameraSystem_GetPosition(void);

void cameraSystem_SetZoom(float zoom);
float cameraSystem_GetZoom(void);

void cameraSystem_SetRotation(float rotation);
float cameraSystem_GetRotation(void);

void cameraSystem_SetMode(CameraSystemMode mode);
CameraSystemMode cameraSystem_GetMode(void);

void cameraSystem_SetTargetEntity(Entity target);
Entity cameraSystem_GetTargetEntity(void);

void cameraSystem_SetSmoothSpeed(float smoothSpeed);
float cameraSystem_GetSmoothSpeed(void);

void cameraSystem_StartShake(float duration, float intensity);

Camera2D cameraSystem_GetInternal(void);
creRectangle cameraSystem_GetViewBounds(void);
creRectangle cameraSystem_GetCullBounds(void);

#endif