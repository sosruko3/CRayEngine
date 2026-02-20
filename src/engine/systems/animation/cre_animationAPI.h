#ifndef CRE_ANIMATIONAPI_H
#define CRE_ANIMATIONAPI_H

#include <stdint.h>
#include <stdbool.h>
typedef struct CommandBus CommandBus;
typedef struct Entity Entity;

void animAPI_Play(CommandBus* restrict bus, Entity entity,uint16_t animID, bool forceReset);
void animAPI_Stop(CommandBus* restrict bus, Entity entity);
void animAPI_Resume(CommandBus* restrict bus, Entity entity);
void animAPI_Pause(CommandBus* restrict bus, Entity entity);
void animAPI_SetSpeed(CommandBus* restrict bus, Entity entity,float speed);
void animAPI_SetFrame(CommandBus* restrict bus, Entity entity, uint16_t frame);
void animAPI_SetLoop(CommandBus* restrict bus, Entity entity, bool loop);
#endif
