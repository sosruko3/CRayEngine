#include "cre_cameraUtils.h"
#include "cre_cameraSystem.h"
#include "engine/core/cre_typesMacro.h"
#include "engine/ecs/cre_entityRegistry.h"
#include <assert.h>
#include <math.h>

Camera2D cameraUtils_buildRaylibCam(const CameraComponent *cam,
                                    ViewportSize vp) {
  assert(cam && "cam is NULL");
  Camera2D out = {};
  // Giving safe values to these so it does not crash if cam is NULL
  out.zoom = 1.0f;
  out.offset = Vector2{vp.width * 0.5f, vp.height * 0.5f};

  if (cam) {
    out.offset = Vector2{vp.width * 0.5f, vp.height * 0.5f};
    out.target = Vector2{cam->viewPosition.x, cam->viewPosition.y};
    out.rotation = cam->rotation * RAD2DEG;
    out.zoom = cam->zoom;
  }
  return out;
}

creVec2 cameraUtils_Lerp(creVec2 current, creVec2 target, float speed,
                         float dt) {
  float t = 1.0f - expf(-speed * dt);

  return creVec2{.x = current.x + (target.x - current.x) * t,
                 .y = current.y + (target.y - current.y) * t};
}

creVec2 cameraUtils_ScreenToWorld(creVec2 screenPos, const EntityRegistry *reg,
                                  const CameraComponent *cam, ViewportSize vp) {
  (void)reg;
  Camera2D rlCam = cameraUtils_buildRaylibCam(cam, vp);
  Vector2 result = GetScreenToWorld2D(R_VEC(screenPos), rlCam);
  return creVec2{result.x, result.y};
}

creVec2 cameraUtils_WorldToScreen(creVec2 worldPos, const EntityRegistry *reg,
                                  const CameraComponent *cam, ViewportSize vp) {
  (void)reg;
  Camera2D rlCam = cameraUtils_buildRaylibCam(cam, vp);
  Vector2 result = GetWorldToScreen2D(R_VEC(worldPos), rlCam);
  return creVec2{result.x, result.y};
}

Camera2D cameraUtils_GetActiveRaylib(const EntityRegistry *reg,
                                     ViewportSize vp) {
  Camera2D out = {};
  if (!reg)
    return out;

  int32_t idx = cameraSystem_FindActive(reg);
  if (idx < 0 || idx >= (int32_t)MAX_CAMERAS)
    return out;

  return cameraUtils_buildRaylibCam(&reg->cameras[idx], vp);
}
