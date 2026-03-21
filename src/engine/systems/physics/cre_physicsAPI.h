#ifndef CRE_PHYSICSAPI_H
#define CRE_PHYSICSAPI_H

#include "engine/core/cre_types.h"
#include <stdbool.h>
#include <stdint.h>

struct CommandBus;
struct EntityRegistry;

void physicsAPI_DefineBody(CommandBus &bus, Entity entity, uint8_t mat_id,
                           float drag, bool is_static);
void physicsAPI_TeleportBody(CommandBus &bus, Entity entity, float x, float y);
void physicsAPI_ApplyImpulse(CommandBus &bus, Entity entity, float jx,
                             float jy);
void physicsAPI_SetVelocity(CommandBus &bus, Entity entity, float vx, float vy);
void physicsAPI_SetDrag(CommandBus &bus, Entity entity, float drag);
void physicsAPI_SetGravityScale(CommandBus &bus, Entity entity, float scale);
void physicsAPI_SetMaterial(CommandBus &bus, Entity entity, uint8_t mat_id);

void physicsAPI_SetGlobalGravity(CommandBus &bus, float x, float y);
void physicsAPI_LoadStaticGeometry(CommandBus &bus);
void physicsAPI_ResetWorld(CommandBus &bus);

creVec2 physicsAPI_GetVelocity(const EntityRegistry &reg, Entity entity);
bool physicsAPI_IsSleeping(const EntityRegistry &reg, Entity entity);
uint8_t physicsAPI_GetMaterial(const EntityRegistry &reg, Entity entity);

#endif
