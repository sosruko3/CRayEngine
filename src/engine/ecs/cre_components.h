#ifndef CRE_COMPONENTS_H
#define CRE_COMPONENTS_H

#include "engine/core/cre_types.h"
#include <stdbool.h>
#include <stdint.h>

struct CameraComponent {
  Entity ownerEntity;

  float zoom;
  float rotation;
  uint16_t priority;
  uint16_t _priority_pad;

  struct {
    creRectangle viewportRect;
    uint32_t cullingMask;
    uint16_t renderTargetID;
    uint16_t _pad0;
  } render;

  struct {
    Entity targetEntity;
    float smoothSpeed;
    creVec2 offset;
    bool enabled;
    uint8_t _pad[3];
  } follow;

  creVec2 viewPosition;

  bool isActive;
  uint8_t _pad[3];
};

CameraComponent cameraSystem_CreateDefault(void);

#endif
