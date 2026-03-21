#ifndef CRE_CAMERAAPI_H
#define CRE_CAMERAAPI_H

#include "engine/core/cre_types.h"
#include <stdbool.h>
#include <stdint.h>
struct CommandBus;
struct Entity;

void cameraAPI_SetActive(CommandBus &bus, Entity cameraEntity, bool isActive);
void cameraAPI_SetPriority(CommandBus &bus, Entity cameraEntity,
                           uint16_t priority);
void cameraAPI_SetZoom(CommandBus &bus, Entity cameraEntity, float zoom);
void cameraAPI_SetRotation(CommandBus &bus, Entity cameraEntity,
                           float rotation);
void cameraAPI_SetFollowTarget(CommandBus &bus, Entity cameraEntity,
                               Entity targetEntity, float smoothSpeed,
                               creVec2 offset);
void cameraAPI_DisableFollow(CommandBus &bus, Entity cameraEntity);

#endif
