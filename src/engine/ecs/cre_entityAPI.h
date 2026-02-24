#ifndef CRE_ENTITYAPI_H
#define CRE_ENTITYAPI_H

#include <stdint.h>

typedef struct Entity Entity;
typedef struct CommandBus CommandBus;
typedef struct EntityRegistry EntityRegistry;
typedef struct creVec2 creVec2;

Entity entityAPI_ReserveSlot(EntityRegistry* reg);
Entity entityAPI_Spawn(EntityRegistry* reg,
					   CommandBus* restrict bus,
					   Entity prototype,
					   creVec2 position);
Entity entityAPI_SpawnUntracked(EntityRegistry* reg,
								CommandBus* restrict bus,
								Entity prototype,
								creVec2 position);

void entityAPI_Destroy(CommandBus* bus, Entity entity);
void entityAPI_AddFlags(CommandBus* bus, Entity entity, uint64_t flags);
void entityAPI_RemoveFlags(CommandBus* bus, Entity entity, uint64_t flags);
void entityAPI_SetType(CommandBus* bus, Entity entity, uint16_t type);
void entityAPI_SetPivot(CommandBus* bus, Entity entity, creVec2 pivot);
void entityAPI_AddComponent(CommandBus* bus, Entity entity, uint64_t component_mask);
void entityAPI_RemoveComponent(CommandBus* bus, Entity entity, uint64_t component_mask);

void entityAPI_Clone(EntityRegistry* reg,
                     CommandBus* restrict bus,
					 Entity dst,
					 Entity prototype,
					 creVec2 position);

#endif
