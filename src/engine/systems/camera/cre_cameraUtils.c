#include "cre_cameraUtils.h"
#include "cre_cameraSystem.h"
#include "engine/core/cre_typesMacro.h"
#include "engine/ecs/cre_entityRegistry.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

static Camera2D buildRaylibCam(const EntityRegistry *reg,
                               const CameraComponent *cam, ViewportSize vp) {
  Camera2D out = {0};

  assert(reg && "reg is NULL");
  assert(cam && "cam is NULL");
  Entity ownEntity = cam->ownerEntity;
  if (!(EntityRegistry_IsAlive(reg, ownEntity))) {
    return out;
  }

  out.offset = (Vector2){vp.width * 0.5f, vp.height * 0.5f};
  out.target = (Vector2){reg->pos_x[ownEntity.id] + cam->shake.currentOffset.x,
                         reg->pos_y[ownEntity.id] + cam->shake.currentOffset.y};
  out.rotation = cam->rotation * RAD2DEG;
  out.zoom = cam->zoom;

  return out;
}

creVec2 cameraUtils_Lerp(creVec2 current, creVec2 target, float speed,
                         float dt) {
  float t = 1.0f - expf(-speed * dt);

  return (creVec2){.x = current.x + (target.x - current.x) * t,
                   .y = current.y + (target.y - current.y) * t};
}

creVec2 cameraUtils_RandomShakeOffset(float intensity) {
  if (intensity <= 0.0f)
    return (creVec2){0.0f, 0.0f};

  float offsetX = ((float)rand() / (float)RAND_MAX * 2.0f - 1.0f) * intensity;
  float offsetY = ((float)rand() / (float)RAND_MAX * 2.0f - 1.0f) * intensity;

  return (creVec2){offsetX, offsetY};
}

creVec2 cameraUtils_ScreenToWorld(creVec2 screenPos, const EntityRegistry *reg,
                                  const CameraComponent *cam, ViewportSize vp) {
  Camera2D rlCam = buildRaylibCam(reg, cam, vp);
  Vector2 result = GetScreenToWorld2D(R_VEC(screenPos), rlCam);
  return (creVec2){result.x, result.y};
}

creVec2 cameraUtils_WorldToScreen(creVec2 worldPos, const EntityRegistry *reg,
                                  const CameraComponent *cam, ViewportSize vp) {
  Camera2D rlCam = buildRaylibCam(reg, cam, vp);
  Vector2 result = GetWorldToScreen2D(R_VEC(worldPos), rlCam);
  return (creVec2){result.x, result.y};
}

Camera2D cameraUtils_GetActiveRaylib(const EntityRegistry *reg,
                                     ViewportSize vp) {
  Camera2D out = {0};
  if (!reg)
    return out;

  int32_t idx = cameraSystem_FindActive(reg);
  if (idx < 0 || idx >= (int32_t)MAX_CAMERAS)
    return out;

  return buildRaylibCam(reg, &reg->cameras[idx], vp);
}
