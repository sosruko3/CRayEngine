#ifndef CRE_ENTITYSYSTEM_H
#define CRE_ENTITYSYSTEM_H

#include <stdbool.h>
#include "cre_entityRegistry.h"
typedef struct CommandBus CommandBus;

bool EntitySystem_SubscribeOnCloned(EntityRegistry* reg, OnEntityClonedCallback cb);
bool EntitySystem_UnsubscribeOnCloned(EntityRegistry* reg, OnEntityClonedCallback cb);
void EntitySystem_ClearCloneHooks(EntityRegistry* reg);

bool EntitySystem_SubscribeOnSpawned(EntityRegistry* reg, OnEntitySpawnedCallback cb);
bool EntitySystem_UnsubscribeOnSpawned(EntityRegistry* reg, OnEntitySpawnedCallback cb);
void EntitySystem_ClearSpawnHooks(EntityRegistry* reg);

bool EntitySystem_SubscribeOnDestroyed(EntityRegistry* reg, OnEntityDestroyedCallback cb);
bool EntitySystem_UnsubscribeOnDestroyed(EntityRegistry* reg, OnEntityDestroyedCallback cb);
void EntitySystem_ClearDestroyHooks(EntityRegistry* reg);

void EntitySystem_ClearAllHooks(EntityRegistry* reg);

void EntitySystem_ProcessCommands(EntityRegistry* reg, CommandBus* bus);

void EntitySystem_Update(EntityRegistry* reg, CommandBus* bus);
#endif
