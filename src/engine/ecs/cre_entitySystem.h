#ifndef CRE_ENTITYSYSTEM_H
#define CRE_ENTITYSYSTEM_h


void EntitySystem_ProcessCommands(EntityRegistry* reg, CommandBus* bus);

void EntitySystem_Update(EntityRegistry* reg, CommandBus* bus);
#endif
