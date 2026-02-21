#include "cre_entitySystem.h"
#include "cre_entityManager.h"
#include "engine/core/cre_commandBus.h"

#include <assert.h>
#include <stdint.h>

#define MAX_CLONE_HOOKS 8

static OnEntityClonedCallback s_clone_hooks[MAX_CLONE_HOOKS] = { NULL };
static uint8_t s_clone_hook_count = 0;

bool EntitySystem_SubscribeOnCloned(OnEntityClonedCallback cb) {
    assert(cb != NULL && "Subscribe callback cannot be NULL!");
    if (cb == NULL) return false;

    for (uint8_t i = 0; i < s_clone_hook_count; ++i) {
        if (s_clone_hooks[i] == cb) return false;
    }

    if (s_clone_hook_count >= MAX_CLONE_HOOKS) return false;

    s_clone_hooks[s_clone_hook_count] = cb;
    s_clone_hook_count++;
    return true;
}

bool EntitySystem_UnsubscribeOnCloned(OnEntityClonedCallback cb) {
    assert(cb != NULL && "Unsubscribe callback cannot be NULL!");
    if (cb == NULL) return false;

    for (uint8_t i = 0; i < s_clone_hook_count; ++i) {
        if (s_clone_hooks[i] != cb) continue;

        for (uint8_t j = i; j + 1 < s_clone_hook_count; ++j) {
            s_clone_hooks[j] = s_clone_hooks[j + 1];
        }

        s_clone_hook_count--;
        s_clone_hooks[s_clone_hook_count] = NULL;
        return true;
    }

    return false;
}

void EntitySystem_ProcessCommands(EntityRegistry* reg, CommandBus* bus) {
    assert(reg && "reg is NULL");
    assert(bus && "Bus is mandatory!");

    CommandIterator iter = CommandBus_GetIterator(bus);
    const Command* cmd;

    while (CommandBus_Next(bus, &iter, &cmd)) {
        if ((cmd->type & CMD_DOMAIN_MASK) != CMD_DOMAIN_ENTITY) continue;
        Entity entity = cmd->entity;
        switch (cmd->type) {
            case CMD_ENTITY_SPAWN: {

                break;
            }
            case CMD_ENTITY_SPAWN_UNTRACKED: {

                break;
            }
            case CMD_ENTITY_CLONE: {

                break;
            }
            case CMD_ENTITY_DESTROY: {

                break;
            }
            case CMD_ENTITY_ADDCOMPONENT: {

                break;
            }
            case CMD_ENTITY_SETPIVOT: {

                break;
            }
            case CMD_ENTITY_SETTYPE: {

                break;
            }
            case CMD_ENTITY_SETFLAGS: {

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