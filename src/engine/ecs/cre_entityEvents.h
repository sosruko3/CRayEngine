#ifndef CRE_ENTITYEVENTS_H
#define CRE_ENTITYEVENTS_H

#include <stdbool.h>
typedef struct EntityRegistry EntityRegistry;
typedef struct CommandBus CommandBus;

#define MAX_CLONE_HOOKS 8
#define MAX_SPAWN_HOOKS 8
#define MAX_DESTROY_HOOKS 8

typedef void (*OnEntityClonedCallback)(EntityRegistry* reg,
                                       CommandBus* bus,
                                       Entity srcPrototype,
                                       Entity dstNewEntity);

typedef void (*OnEntitySpawnedCallback)(EntityRegistry* reg,
                                        CommandBus* bus,
                                        Entity srcPrototype,
                                        Entity dstNewEntity);

typedef void (*OnEntityDestroyedCallback)(EntityRegistry* reg,
                                          CommandBus* bus,
                                          Entity destroyedEntity);

typedef struct EntityEventDispatcher {
    OnEntityClonedCallback clone_hooks[MAX_CLONE_HOOKS];
    uint8_t clone_hook_count;
    bool is_dispatching_clone_hooks;

    OnEntitySpawnedCallback spawn_hooks[MAX_SPAWN_HOOKS];
    uint8_t spawn_hook_count;
    bool is_dispatching_spawn_hooks;

    OnEntityDestroyedCallback destroy_hooks[MAX_DESTROY_HOOKS];
    uint8_t destroy_hook_count;
    bool is_dispatching_destroy_hooks;
} EntityEventDispatcher;

#endif