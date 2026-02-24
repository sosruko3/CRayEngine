#include "cre_entitySystem.h"
#include "cre_entityManager.h"
#include "cre_entityRegistry.h"
#include "engine/core/cre_commandBus.h"
#include "engine/core/cre_logger.h"

#include <assert.h>
#include <stdint.h>

static void EntitySystem_CopyPrototype(EntityRegistry* reg,
                                       uint32_t dst_id,
                                       uint32_t src_id,
                                       creVec2 position) {
    reg->pos_x[dst_id] = reg->pos_x[src_id];
    reg->pos_y[dst_id] = reg->pos_y[src_id];
    reg->size_w[dst_id] = reg->size_w[src_id];
    reg->size_h[dst_id] = reg->size_h[src_id];
    reg->rotation[dst_id] = reg->rotation[src_id];

    reg->component_masks[dst_id] = reg->component_masks[src_id];
    reg->state_flags[dst_id] = reg->state_flags[src_id] & ~CLONE_FLAGS_SCRUB_MASK;
    reg->types[dst_id] = reg->types[src_id];

    reg->render_layer[dst_id] = reg->render_layer[src_id];
    reg->batch_ids[dst_id] = reg->batch_ids[src_id];
    reg->sprite_ids[dst_id] = reg->sprite_ids[src_id];
    reg->colors[dst_id] = reg->colors[src_id];
    reg->pivot_x[dst_id] = reg->pivot_x[src_id];
    reg->pivot_y[dst_id] = reg->pivot_y[src_id];

    reg->material_id[dst_id] = reg->material_id[src_id];
    reg->drag[dst_id] = reg->drag[src_id];
    reg->inv_mass[dst_id] = reg->inv_mass[src_id];
    reg->gravity_scale[dst_id] = reg->gravity_scale[src_id];

    reg->anim_speeds[dst_id] = reg->anim_speeds[src_id];
    reg->anim_ids[dst_id] = reg->anim_ids[src_id];
    reg->anim_base_durations[dst_id] = reg->anim_base_durations[src_id];
    reg->anim_frame_counts[dst_id] = reg->anim_frame_counts[src_id];
    reg->anim_start_sprites[dst_id] = reg->anim_start_sprites[src_id];
    reg->anim_loops[dst_id] = reg->anim_loops[src_id];

    reg->pos_x[dst_id] = position.x;
    reg->pos_y[dst_id] = position.y;

    reg->anim_timers[dst_id] = 0.0f;
    reg->anim_frames[dst_id] = 0;
    reg->anim_finished[dst_id] = false;

    reg->vel_x[dst_id] = 0.0f;
    reg->vel_y[dst_id] = 0.0f;

    reg->state_flags[dst_id] |= FLAG_ACTIVE;

    if (dst_id >= reg->max_used_bound) {
        reg->max_used_bound = dst_id + 1;
    }

    reg->active_count++;
    assert(reg->active_count <= MAX_ENTITIES);
}

bool EntitySystem_SubscribeOnCloned(EntityRegistry* reg, OnEntityClonedCallback cb) {
    assert(reg != NULL && "reg is NULL");
    assert(cb != NULL && "Subscribe callback cannot be NULL!");
    assert(!reg->events.is_dispatching_clone_hooks && "Cannot subscribe during dispatch loop");

    if (reg == NULL || cb == NULL) return false;

    for (uint8_t i = 0; i < reg->events.clone_hook_count; ++i) {
        if (reg->events.clone_hooks[i] == cb) return false;
    }

    if (reg->events.clone_hook_count >= MAX_CLONE_HOOKS) {
        Log(LOG_LVL_ERROR, "EntitySystem_SubscribeOnCloned: Hook capacity exceeded (%u)", MAX_CLONE_HOOKS);
        assert(false && "EntitySystem_SubscribeOnCloned: Hook capacity exceeded");
        return false;
    }

    reg->events.clone_hooks[reg->events.clone_hook_count++] = cb;
    return true;
}

bool EntitySystem_UnsubscribeOnCloned(EntityRegistry* reg, OnEntityClonedCallback cb) {
    assert(reg != NULL && "reg is NULL");
    assert(cb != NULL && "Unsubscribe callback cannot be NULL!");
    assert(!reg->events.is_dispatching_clone_hooks && "Cannot unsubscribe during dispatch loop");

    if (reg == NULL || cb == NULL) return false;

    for (uint8_t i = 0; i < reg->events.clone_hook_count; ++i) {
        if (reg->events.clone_hooks[i] != cb) continue;

        for (uint8_t j = i; j + 1 < reg->events.clone_hook_count; ++j) {
            reg->events.clone_hooks[j] = reg->events.clone_hooks[j + 1];
        }

        reg->events.clone_hook_count--;
        reg->events.clone_hooks[reg->events.clone_hook_count] = NULL;
        return true;
    }

    return false;
}

void EntitySystem_ClearCloneHooks(EntityRegistry* reg) {
    assert(reg != NULL && "reg is NULL");
    assert(!reg->events.is_dispatching_clone_hooks && "Cannot clear hooks during dispatch loop");
    if (reg == NULL) return;

    for (uint8_t i = 0; i < MAX_CLONE_HOOKS; ++i) {
        reg->events.clone_hooks[i] = NULL;
    }

    reg->events.clone_hook_count = 0;
    reg->events.is_dispatching_clone_hooks = false;
}

bool EntitySystem_SubscribeOnSpawned(EntityRegistry* reg, OnEntitySpawnedCallback cb) {
    assert(reg != NULL && "reg is NULL");
    assert(cb != NULL && "Subscribe callback cannot be NULL!");
    assert(!reg->events.is_dispatching_spawn_hooks && "Cannot subscribe during dispatch loop");

    if (reg == NULL || cb == NULL) return false;

    for (uint8_t i = 0; i < reg->events.spawn_hook_count; ++i) {
        if (reg->events.spawn_hooks[i] == cb) return false;
    }

    if (reg->events.spawn_hook_count >= MAX_SPAWN_HOOKS) {
        Log(LOG_LVL_ERROR, "EntitySystem_SubscribeOnSpawned: Hook capacity exceeded (%u)", MAX_SPAWN_HOOKS);
        assert(false && "EntitySystem_SubscribeOnSpawned: Hook capacity exceeded");
        return false;
    }

    reg->events.spawn_hooks[reg->events.spawn_hook_count++] = cb;
    return true;
}

bool EntitySystem_UnsubscribeOnSpawned(EntityRegistry* reg, OnEntitySpawnedCallback cb) {
    assert(reg != NULL && "reg is NULL");
    assert(cb != NULL && "Unsubscribe callback cannot be NULL!");
    assert(!reg->events.is_dispatching_spawn_hooks && "Cannot unsubscribe during dispatch loop");

    if (reg == NULL || cb == NULL) return false;

    for (uint8_t i = 0; i < reg->events.spawn_hook_count; ++i) {
        if (reg->events.spawn_hooks[i] != cb) continue;

        for (uint8_t j = i; j + 1 < reg->events.spawn_hook_count; ++j) {
            reg->events.spawn_hooks[j] = reg->events.spawn_hooks[j + 1];
        }

        reg->events.spawn_hook_count--;
        reg->events.spawn_hooks[reg->events.spawn_hook_count] = NULL;
        return true;
    }

    return false;
}

void EntitySystem_ClearSpawnHooks(EntityRegistry* reg) {
    assert(reg != NULL && "reg is NULL");
    assert(!reg->events.is_dispatching_spawn_hooks && "Cannot clear hooks during dispatch loop");
    if (reg == NULL) return;

    for (uint8_t i = 0; i < MAX_SPAWN_HOOKS; ++i) {
        reg->events.spawn_hooks[i] = NULL;
    }

    reg->events.spawn_hook_count = 0;
    reg->events.is_dispatching_spawn_hooks = false;
}

bool EntitySystem_SubscribeOnDestroyed(EntityRegistry* reg, OnEntityDestroyedCallback cb) {
    assert(reg != NULL && "reg is NULL");
    assert(cb != NULL && "Subscribe callback cannot be NULL!");
    assert(!reg->events.is_dispatching_destroy_hooks && "Cannot subscribe during dispatch loop");

    if (reg == NULL || cb == NULL) return false;

    for (uint8_t i = 0; i < reg->events.destroy_hook_count; ++i) {
        if (reg->events.destroy_hooks[i] == cb) return false;
    }

    if (reg->events.destroy_hook_count >= MAX_DESTROY_HOOKS) {
        Log(LOG_LVL_ERROR, "EntitySystem_SubscribeOnDestroyed: Hook capacity exceeded (%u)", MAX_DESTROY_HOOKS);
        assert(false && "EntitySystem_SubscribeOnDestroyed: Hook capacity exceeded");
        return false;
    }

    reg->events.destroy_hooks[reg->events.destroy_hook_count++] = cb;
    return true;
}

bool EntitySystem_UnsubscribeOnDestroyed(EntityRegistry* reg, OnEntityDestroyedCallback cb) {
    assert(reg != NULL && "reg is NULL");
    assert(cb != NULL && "Unsubscribe callback cannot be NULL!");
    assert(!reg->events.is_dispatching_destroy_hooks && "Cannot unsubscribe during dispatch loop");

    if (reg == NULL || cb == NULL) return false;

    for (uint8_t i = 0; i < reg->events.destroy_hook_count; ++i) {
        if (reg->events.destroy_hooks[i] != cb) continue;

        for (uint8_t j = i; j + 1 < reg->events.destroy_hook_count; ++j) {
            reg->events.destroy_hooks[j] = reg->events.destroy_hooks[j + 1];
        }

        reg->events.destroy_hook_count--;
        reg->events.destroy_hooks[reg->events.destroy_hook_count] = NULL;
        return true;
    }

    return false;
}

void EntitySystem_ClearDestroyHooks(EntityRegistry* reg) {
    assert(reg != NULL && "reg is NULL");
    assert(!reg->events.is_dispatching_destroy_hooks && "Cannot clear hooks during dispatch loop");
    if (reg == NULL) return;

    for (uint8_t i = 0; i < MAX_DESTROY_HOOKS; ++i) {
        reg->events.destroy_hooks[i] = NULL;
    }

    reg->events.destroy_hook_count = 0;
    reg->events.is_dispatching_destroy_hooks = false;
}

void EntitySystem_ClearAllHooks(EntityRegistry* reg) {
    EntitySystem_ClearCloneHooks(reg);
    EntitySystem_ClearSpawnHooks(reg);
    EntitySystem_ClearDestroyHooks(reg);
}

void EntitySystem_ProcessCommands(EntityRegistry* reg, CommandBus* bus) {
    assert(reg && "reg is NULL");
    assert(bus && "Bus is mandatory!");

    CommandIterator iter = CommandBus_GetIterator(bus);
    const Command* cmd;

    while (CommandBus_Next(bus, &iter, &cmd)) {
        if ((cmd->type & CMD_DOMAIN_MASK) != CMD_DOMAIN_ENTITY) continue;

        switch (cmd->type) {
            case CMD_ENTITY_SPAWN:
            case CMD_ENTITY_SPAWN_UNTRACKED: {
                const Entity dst = cmd->entity;
                const Entity src = cmd->entityClone.prototype;
                const creVec2 spawnPos = cmd->entityClone.position;

                if (dst.id == src.id) {
                    EntityManager_ReturnReservedSlot(reg, dst);
                    break;
                }

                if (dst.id >= MAX_ENTITIES || src.id >= MAX_ENTITIES) {
                    if (dst.id < MAX_ENTITIES) EntityManager_ReturnReservedSlot(reg, dst);
                    break;
                }

                if (reg->state_flags[dst.id] & FLAG_ACTIVE) {
                    EntityManager_ReturnReservedSlot(reg, dst);
                    break;
                }

                if (reg->generations[dst.id] != dst.generation) {
                    EntityManager_ReturnReservedSlot(reg, dst);
                    break;
                }

                if (!EntityRegistry_IsAlive(reg, src)) {
                    EntityManager_ReturnReservedSlot(reg, dst);
                    break;
                }

                EntitySystem_CopyPrototype(reg, dst.id, src.id, spawnPos);

                if (cmd->type == CMD_ENTITY_SPAWN) {
                    reg->events.is_dispatching_spawn_hooks = true;
                    const uint8_t hook_count = reg->events.spawn_hook_count;
                    for (uint8_t i = 0; i < hook_count; ++i) {
                        OnEntitySpawnedCallback hook = reg->events.spawn_hooks[i];
                        if (hook == NULL) continue;
                        hook(reg, bus, src, dst);
                    }
                    reg->events.is_dispatching_spawn_hooks = false;
                }

                break;
            }
            case CMD_ENTITY_CLONE: {
                const Entity dst = cmd->entity;
                const Entity src = cmd->entityClone.prototype;
                const creVec2 spawnPos = cmd->entityClone.position;

                if (dst.id == src.id) {
                    EntityManager_ReturnReservedSlot(reg, dst);
                    break;
                }

                if (dst.id >= MAX_ENTITIES || src.id >= MAX_ENTITIES) {
                    if (dst.id < MAX_ENTITIES) {
                        EntityManager_ReturnReservedSlot(reg, dst);
                    }
                    break;
                }

                if (reg->state_flags[dst.id] & FLAG_ACTIVE) {
                    EntityManager_ReturnReservedSlot(reg, dst);
                    break;
                }

                if (reg->generations[dst.id] != dst.generation) {
                    EntityManager_ReturnReservedSlot(reg, dst);
                    break;
                }

                if (!EntityRegistry_IsAlive(reg, src)) {
                    EntityManager_ReturnReservedSlot(reg, dst);
                    break;
                }

                EntitySystem_CopyPrototype(reg, dst.id, src.id, spawnPos);

                reg->events.is_dispatching_clone_hooks = true;
                const uint8_t hook_count = reg->events.clone_hook_count;
                for (uint8_t i = 0; i < hook_count; ++i) {
                    OnEntityClonedCallback hook = reg->events.clone_hooks[i];
                    if (hook == NULL) continue;
                    hook(reg, bus, src, dst);
                }
                reg->events.is_dispatching_clone_hooks = false;
                break;
            }
            case CMD_ENTITY_DESTROY: {
                if (!EntityRegistry_IsAlive(reg, cmd->entity)) break;
                uint32_t id = cmd->entity.id;

                reg->events.is_dispatching_destroy_hooks = true;
                const uint8_t hook_count = reg->events.destroy_hook_count;
                for (uint8_t i = 0; i < hook_count; ++i) {
                    OnEntityDestroyedCallback hook = reg->events.destroy_hooks[i];
                    if (hook == NULL) continue;
                    hook(reg, bus, cmd->entity);
                }
                reg->events.is_dispatching_destroy_hooks = false;

                reg->component_masks[id] = COMP_NONE;
                reg->state_flags[id] &= ~FLAG_ACTIVE;
                reg->generations[id]++;
                reg->free_list[reg->free_count++] = id;
                reg->active_count--;
                break;
            }
            case CMD_ENTITY_ADD_COMPONENT: {
                if (!EntityRegistry_IsAlive(reg, cmd->entity)) break;
                uint32_t id = cmd->entity.id;
                reg->component_masks[id] |= cmd->u64.value;
                break;
            }
            case CMD_ENTITY_REMOVE_COMPONENT: {
                if (!EntityRegistry_IsAlive(reg, cmd->entity)) break;
                uint32_t id = cmd->entity.id;
                reg->component_masks[id] &= ~cmd->u64.value;
                break;
            }
            case CMD_ENTITY_SET_PIVOT: {
                if (!EntityRegistry_IsAlive(reg, cmd->entity)) break;
                uint32_t id = cmd->entity.id;
                reg->pivot_x[id] = cmd->vec2.value.x;
                reg->pivot_y[id] = cmd->vec2.value.y;
                break;
            }
            case CMD_ENTITY_SET_TYPE: {
                if (!EntityRegistry_IsAlive(reg, cmd->entity)) break;
                uint32_t id = cmd->entity.id;
                reg->types[id] = cmd->u16.value;
                break;
            }
            case CMD_ENTITY_ADD_FLAGS: { 
                // Change it's name to ADD_FLAGS
                if (!EntityRegistry_IsAlive(reg, cmd->entity)) break;
                uint32_t id = cmd->entity.id;
                reg->state_flags[id] |= cmd->u64.value;
                break;
            }
            case CMD_ENTITY_REMOVE_FLAGS: {
                if (!EntityRegistry_IsAlive(reg, cmd->entity)) break;
                uint32_t id = cmd->entity.id;
                reg->state_flags[id] &= ~cmd->u64.value;
                break;
            }
            case CMD_ENTITY_RESET: {
                break;
            }
            default:
                break;
        }
    }
}

void EntitySystem_Update(EntityRegistry* reg, CommandBus* bus) {
    EntitySystem_ProcessCommands(reg, bus);
}