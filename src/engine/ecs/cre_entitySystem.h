#ifndef CRE_ENTITYSYSTEM_H
#define CRE_ENTITYSYSTEM_H

typedef struct EntityRegistry EntityRegistry;
typedef struct CommandBus CommandBus;
void EntitySystem_ProcessCommands(EntityRegistry* reg, CommandBus* bus);

void EntitySystem_Update(EntityRegistry* reg, CommandBus* bus);
#endif
