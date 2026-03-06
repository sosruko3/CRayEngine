#ifndef CRE_PHYSICSAPI_H
#define CRE_PHYSICSAPI_H

#include "engine/core/cre_types.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct Entity Entity;
typedef struct CommandBus CommandBus;
typedef struct EntityRegistry EntityRegistry;

void physicsAPI_DefineBody(CommandBus* restrict bus, Entity entity, uint8_t mat_id, float drag, bool is_static);
void physicsAPI_TeleportBody(CommandBus* restrict bus, Entity entity, float x, float y);
void physicsAPI_ApplyImpulse(CommandBus* restrict bus, Entity entity, float jx, float jy);
void physicsAPI_SetVelocity(CommandBus* restrict bus, Entity entity, float vx, float vy);
void physicsAPI_SetDrag(CommandBus* restrict bus, Entity entity, float drag);
void physicsAPI_SetGravityScale(CommandBus* restrict bus, Entity entity, float scale);
void physicsAPI_SetMaterial(CommandBus* restrict bus, Entity entity, uint8_t mat_id);

void physicsAPI_SetGlobalGravity(CommandBus* restrict bus, float x, float y);
void physicsAPI_LoadStaticGeometry(CommandBus* restrict bus);
void physicsAPI_ResetWorld(CommandBus* restrict bus);

creVec2 physicsAPI_GetVelocity(const EntityRegistry* reg, Entity entity);
bool physicsAPI_IsSleeping(const EntityRegistry* reg, Entity entity);
uint8_t physicsAPI_GetMaterial(const EntityRegistry* reg, Entity entity);

#endif
