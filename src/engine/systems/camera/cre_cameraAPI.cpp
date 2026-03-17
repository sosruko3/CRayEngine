#include "cre_cameraAPI.h"
#include "engine/core/cre_commandBus.h"
#include <assert.h>

void cameraAPI_SetActive(CommandBus *restrict bus, Entity cameraEntity,
                         bool isActive) {
  assert(bus != NULL);
  Command cmd = {
      .type = CMD_CAM_SET_ACTIVE,
      .entity = cameraEntity,
      .b8 = {.value = isActive},
  };
  CommandBus_Push(bus, cmd);
}

void cameraAPI_SetPriority(CommandBus *restrict bus, Entity cameraEntity,
                           uint16_t priority) {
  assert(bus != NULL);
  Command cmd = {
      .type = CMD_CAM_SET_PRIORITY,
      .entity = cameraEntity,
      .u16 = {.value = priority},
  };
  CommandBus_Push(bus, cmd);
}

void cameraAPI_SetZoom(CommandBus *restrict bus, Entity cameraEntity,
                       float zoom) {
  assert(bus != NULL);
  Command cmd = {
      .type = CMD_CAM_SET_ZOOM,
      .entity = cameraEntity,
      .f32 = {.value = zoom},
  };
  CommandBus_Push(bus, cmd);
}

void cameraAPI_SetRotation(CommandBus *restrict bus, Entity cameraEntity,
                           float rotation) {
  assert(bus != NULL);
  Command cmd = {
      .type = CMD_CAM_SET_ROTATION,
      .entity = cameraEntity,
      .f32 = {.value = rotation},
  };
  CommandBus_Push(bus, cmd);
}

void cameraAPI_SetFollowTarget(CommandBus *restrict bus, Entity cameraEntity,
                               Entity targetEntity, float smoothSpeed,
                               creVec2 offset) {
  assert(bus != NULL);
  Command cmd = {
      .type = CMD_CAM_SET_FOLLOW,
      .entity = cameraEntity,
      .camFollow =
          {
              .targetEntity = targetEntity,
              .smoothSpeed = smoothSpeed,
              .offset = offset,
          },
  };
  CommandBus_Push(bus, cmd);
}

void cameraAPI_DisableFollow(CommandBus *restrict bus, Entity cameraEntity) {
  assert(bus != NULL);
  Command cmd = {
      .type = CMD_CAM_DISABLE_FOLLOW,
      .entity = cameraEntity,
      .u16 = {},
  };
  CommandBus_Push(bus, cmd);
}
