#ifndef CRE_ENTITYAPI_H
#define CRE_ENTITYAPI_H

#include "engine/core/cre_types.h"
#include <stdint.h>

struct CommandBus;
struct EntityRegistry;

Entity entityAPI_ReserveSlot(EntityRegistry &reg);
Entity entityAPI_Spawn(EntityRegistry &reg, CommandBus &bus, Entity prototype,
                       creVec2 position);
Entity entityAPI_SpawnUntracked(EntityRegistry &reg, CommandBus &bus,
                                Entity prototype, creVec2 position);

void entityAPI_Destroy(CommandBus &bus, Entity entity);
void entityAPI_AddFlags(CommandBus &bus, Entity entity, uint64_t flags);
void entityAPI_RemoveFlags(CommandBus &bus, Entity entity, uint64_t flags);
void entityAPI_SetType(CommandBus &bus, Entity entity, uint16_t type);
void entityAPI_SetPivot(CommandBus &bus, Entity entity, creVec2 pivot);
void entityAPI_AddComponent(CommandBus &bus, Entity entity,
                            uint64_t component_mask);
void entityAPI_RemoveComponent(CommandBus &bus, Entity entity,
                               uint64_t component_mask);

void entityAPI_Clone(EntityRegistry &reg, CommandBus &bus, Entity dst,
                     Entity prototype, creVec2 position);

#endif
