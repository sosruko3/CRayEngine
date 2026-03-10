#include "cre_cameraSystem.h"
#include "cre_cameraUtils.h"
#include "engine/core/cre_commandBus.h"
#include "engine/core/cre_config.h"
#include "engine/core/cre_types.h"
#include "engine/ecs/cre_entityRegistry.h"
#include <assert.h>
#include <math.h>

#define cam_safety_epsilon 0.0001f

CameraComponent cameraSystem_CreateDefault(void) {
  CameraComponent cam = {0};
  cam.ownerEntity = ENTITY_INVALID;
  cam.zoom = 1.0f;
  cam.rotation = 0.0f;
  cam.priority = 0;
  cam.follow.targetEntity = ENTITY_INVALID;
  cam.follow.smoothSpeed = 10.0f;
  cam.isActive = false;
  return cam;
}

void cameraSystem_Init(EntityRegistry *reg) {
  assert(reg && "reg is NULL");
  reg->camera_count = 0;
}

void cameraSystem_ProcessCommands(EntityRegistry *reg, CommandBus *bus) {
  CommandIterator iter = CommandBus_GetIterator(bus);
  const Command *cmd;

  while (CommandBus_Next(bus, &iter, &cmd)) {
    switch (cmd->type) {
    default:
      break;
    }
  }
}

static void applyFollowLogic(CameraComponent *cam, EntityRegistry *reg,
                             float dt, uint32_t ownerId) {
  if (EntityRegistry_IsAlive(reg, cam->follow.targetEntity)) {
    const uint32_t targetId = cam->follow.targetEntity.id;
    const creVec2 desired = {
        reg->pos_x[targetId] + cam->follow.offset.x,
        reg->pos_y[targetId] + cam->follow.offset.y,
    };

    creVec2 current = {
        reg->pos_x[ownerId],
        reg->pos_y[ownerId],
    };

    creVec2 nextPos = desired;
    if (cam->follow.smoothSpeed > cam_safety_epsilon) {
      nextPos = cameraUtils_Lerp(current, desired, cam->follow.smoothSpeed, dt);
    }

    reg->pos_x[ownerId] = nextPos.x;
    reg->pos_y[ownerId] = nextPos.y;
  } else {
    cam->follow.enabled = false;
    cam->follow.targetEntity = ENTITY_INVALID;
  }
}

static void applyShakeLogic(CameraComponent *cam, float dt) {
  if (cam->shake.timer > 0) {
    cam->shake.currentOffset =
        cameraUtils_RandomShakeOffset(cam->shake.intensity);
    cam->shake.timer -= dt;
  } else {
    cam->shake.timer = 0.0f;
    cam->shake.intensity = 0.0f;
    cam->shake.currentOffset = (creVec2){0.0f, 0.0f};
  }
}

void cameraSystem_Update(EntityRegistry *reg, CommandBus *bus, float dt,
                         ViewportSize vp) {
  assert(reg && "reg is NULL");
  assert(bus && "bus is NULL");
  (void)vp;

  if (dt > 0.05f)
    dt = 0.05f;
  if (dt < cam_safety_epsilon)
    dt = 0.0f;

  cameraSystem_ProcessCommands(reg, bus);

  uint32_t cam_count = reg->camera_count;

  for (uint32_t i = 0; i < cam_count; i++) {
    CameraComponent *cam = &reg->cameras[i];
    if (!(EntityRegistry_IsAlive(reg, cam->ownerEntity)))
      continue;

    Entity ownEntity = cam->ownerEntity;
    const uint32_t ownerId = ownEntity.id;

    // Follow logic
    if (cam->follow.enabled)
      applyFollowLogic(cam, reg, dt, ownerId);
    // Shake applyShakeLogic
    cam->shake.currentOffset = (creVec2){0.0f, 0.0f};
    if (cam->shake.timer > cam_safety_epsilon)
      applyShakeLogic(cam, dt);
  }
}

creRectangle cameraSystem_GetViewBounds(const EntityRegistry *reg,
                                        const CameraComponent *cam,
                                        ViewportSize vp) {
  assert(reg && "reg is NULL");
  assert(cam && "cam is NULL");

  Entity ownEntity = cam->ownerEntity;
  if (!(EntityRegistry_IsAlive(reg, ownEntity))) {
    return (creRectangle){0.0f, 0.0f, 0.0f, 0.0f};
  }

  float zoom = cam->zoom;
  if (zoom < MIN_ZOOM)
    zoom = MIN_ZOOM;
  if (zoom > MAX_ZOOM)
    zoom = MAX_ZOOM;

  const uint32_t ownerId = ownEntity.id;
  const float camX = reg->pos_x[ownerId];
  const float camY = reg->pos_y[ownerId];

  float viewWidth = vp.width / zoom;
  float viewHeight = vp.height / zoom;

  creRectangle bounds = {.x = camX - (viewWidth * 0.5f),
                         .y = camY - (viewHeight * 0.5f),
                         .width = viewWidth,
                         .height = viewHeight};

  if (fabsf(cam->rotation) > cam_safety_epsilon) {
    const float viewportDiagonal =
        sqrtf((vp.width * vp.width) + (vp.height * vp.height));
    const float visibleDiagonal = viewportDiagonal / zoom;

    bounds.x = camX - (visibleDiagonal * 0.5f);
    bounds.y = camY - (visibleDiagonal * 0.5f);
    bounds.width = visibleDiagonal;
    bounds.height = visibleDiagonal;
  }

  return bounds;
}

creRectangle cameraSystem_GetCullBounds(const EntityRegistry *reg,
                                        const CameraComponent *cam,
                                        ViewportSize vp) {
  creRectangle view = cameraSystem_GetViewBounds(reg, cam, vp);

  return (creRectangle){.x = view.x - CAMERA_CULL_MARGIN,
                        .y = view.y - CAMERA_CULL_MARGIN,
                        .width = view.width + (CAMERA_CULL_MARGIN * 2.0f),
                        .height = view.height + (CAMERA_CULL_MARGIN * 2.0f)};
}

int32_t cameraSystem_FindActive(const EntityRegistry *reg) {
  assert(reg && "reg is NULL");

  int32_t activeIndex = -1;
  int32_t highestPriority = INT32_MIN;

  for (uint32_t i = 0; i < reg->camera_count; i++) {
    const CameraComponent *cam = &reg->cameras[i];
    Entity ownEntity = cam->ownerEntity;
    if (!cam->isActive || !EntityRegistry_IsAlive(reg, ownEntity))
      continue;

    if (cam->priority > highestPriority) {
      highestPriority = cam->priority;
      activeIndex = (int32_t)i;
    }
  }

  return activeIndex;
}
