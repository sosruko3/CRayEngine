#ifndef CRE_CAMERAAPI_H
#define CRE_CAMERAAPI_H

#include "engine/core/cre_types.h"
#include <stdbool.h>
#include <stdint.h>
typedef struct CommandBus CommandBus;
typedef struct Entity Entity;


void cameraAPI_SetActive(CommandBus *restrict bus, Entity cameraEntity,
						 bool isActive);
void cameraAPI_SetPriority(CommandBus *restrict bus, Entity cameraEntity,
						   uint16_t priority);
void cameraAPI_SetZoom(CommandBus *restrict bus, Entity cameraEntity, float zoom);
void cameraAPI_SetRotation(CommandBus *restrict bus, Entity cameraEntity,
						   float rotation);
void cameraAPI_SetFollowTarget(CommandBus *restrict bus, Entity cameraEntity,
							   Entity targetEntity, float smoothSpeed,
							   creVec2 offset);
void cameraAPI_DisableFollow(CommandBus *restrict bus, Entity cameraEntity);

#endif
