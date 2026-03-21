#ifndef CRE_ANIMATIONAPI_H
#define CRE_ANIMATIONAPI_H

#include "engine/core/cre_types.h"
#include <stdbool.h>
#include <stdint.h>
struct CommandBus;

void animAPI_Play(CommandBus &bus, Entity entity, uint16_t animID,
                  bool forceReset);
void animAPI_Stop(CommandBus &bus, Entity entity);
void animAPI_Resume(CommandBus &bus, Entity entity);
void animAPI_Pause(CommandBus &bus, Entity entity);
void animAPI_SetSpeed(CommandBus &bus, Entity entity, float speed);
void animAPI_SetFrame(CommandBus &bus, Entity entity, uint16_t frame);
void animAPI_SetLoop(CommandBus &bus, Entity entity, bool loop);
#endif
