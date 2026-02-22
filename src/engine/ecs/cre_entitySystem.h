#ifndef CRE_ENTITYSYSTEM_H
#define CRE_ENTITYSYSTEM_H

#include <stdbool.h>
#include "cre_entityRegistry.h"
typedef struct CommandBus CommandBus;

bool EntitySystem_SubscribeOnCloned(EntityRegistry* reg, OnEntityClonedCallback cb);
bool EntitySystem_UnsubscribeOnCloned(EntityRegistry* reg, OnEntityClonedCallback cb);
void EntitySystem_ClearCloneHooks(EntityRegistry* reg);

void EntitySystem_ProcessCommands(EntityRegistry* reg, CommandBus* bus);

void EntitySystem_Update(EntityRegistry* reg, CommandBus* bus);
#endif
