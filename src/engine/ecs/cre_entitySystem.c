#include "cre_entitySystem.h"
#include "cre_entityManager.h"
#include "engine/core/cre_commandBus.h"

void EntitySystem_ProcessCommands(EntityRegistry* reg, CommandBus* bus) {
    assert(reg && "reg is NULL");
    assert(bus && "Bus is mandatory!");

    CommandIterator iter = CommandBus_GetIterator(bus);
    const Command* cmd;

    while (CommandBus_Next(bus, &iter, &cmd)) {
        if (cmd->type < CMD_ENTITY_SPAWN || cmd->type > CMD_ENTITY_ADD_COMPONENT) continue;
        Entity entity = cmd->entity;
        const uint32_t id = entity.id;
        switch (cmd->type) {
            case CMD_ENTITY_SPAWN: {
                break;
            }
            case CMD_ENTITY_DESTROY: {

                break;
            }
            case CMD_ENTITY_SET_PIVOT: {

                break;
            }

            case CMD_ENTITY_ADD_COMPONENT: {

                break;
            }
            default:

                break;
        }
    }
}

void EntitySystem_Update(EntityRegistry* reg, CommandBus* bus) {
    EntitySystem_ProcessCommands(reg,bus);
}