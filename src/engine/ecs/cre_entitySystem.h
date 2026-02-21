#ifndef CRE_ENTITYSYSTEM_H
#define CRE_ENTITYSYSTEM_H

#include <stdbool.h>
#include "engine/core/cre_types.h"

typedef struct EntityRegistry EntityRegistry;
typedef struct CommandBus CommandBus;

typedef void (*OnEntityClonedCallback)(EntityRegistry* reg,
									   Entity srcPrototype,
									   Entity dstNewEntity);

bool EntitySystem_SubscribeOnCloned(OnEntityClonedCallback cb);
bool EntitySystem_UnsubscribeOnCloned(OnEntityClonedCallback cb);

void EntitySystem_ProcessCommands(EntityRegistry* reg, CommandBus* bus);

void EntitySystem_Update(EntityRegistry* reg, CommandBus* bus);
#endif
