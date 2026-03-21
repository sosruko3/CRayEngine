#include "cre_cameraAPI.h"
#include "engine/core/cre_commandBus.h"
#include <assert.h>

void cameraAPI_SetActive(CommandBus &bus, Entity cameraEntity, bool isActive) {
  Command cmd = {
      .type = CMD_CAM_SET_ACTIVE,
      .entity = cameraEntity,
      .b8 = {.value = isActive},
  };
  CommandBus_Push(bus, cmd);
}

void cameraAPI_SetPriority(CommandBus &bus, Entity cameraEntity,
                           uint16_t priority) {
  Command cmd = {
      .type = CMD_CAM_SET_PRIORITY,
      .entity = cameraEntity,
      .u16 = {.value = priority},
  };
  CommandBus_Push(bus, cmd);
}

void cameraAPI_SetZoom(CommandBus &bus, Entity cameraEntity, float zoom) {
  Command cmd = {
      .type = CMD_CAM_SET_ZOOM,
      .entity = cameraEntity,
      .f32 = {.value = zoom},
  };
  CommandBus_Push(bus, cmd);
}

void cameraAPI_SetRotation(CommandBus &bus, Entity cameraEntity,
                           float rotation) {
  Command cmd = {
      .type = CMD_CAM_SET_ROTATION,
      .entity = cameraEntity,
      .f32 = {.value = rotation},
  };
  CommandBus_Push(bus, cmd);
}

void cameraAPI_SetFollowTarget(CommandBus &bus, Entity cameraEntity,
                               Entity targetEntity, float smoothSpeed,
                               creVec2 offset) {
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

void cameraAPI_DisableFollow(CommandBus &bus, Entity cameraEntity) {
  Command cmd = {
      .type = CMD_CAM_DISABLE_FOLLOW,
      .entity = cameraEntity,
      .u16 = {},
  };
  CommandBus_Push(bus, cmd);
}
