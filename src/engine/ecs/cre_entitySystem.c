#include "cre_entitySystem.h"
#include "cre_entityManager.h"
#include "cre_entityRegistry.h"
#include "engine/core/cre_commandBus.h"
#include "engine/core/cre_logger.h"

#include <assert.h>
#include <stdint.h>

bool EntitySystem_SubscribeOnCloned(EntityRegistry* reg, OnEntityClonedCallback cb) {
    assert(reg != NULL && "reg is NULL");
    assert(cb != NULL && "Subscribe callback cannot be NULL!");
    assert(!reg->is_dispatching_clone_hooks && "Cannot subscribe during dispatch loop");

    if (reg == NULL) return false;
    if (cb == NULL) return false;

    for (uint8_t i = 0; i < reg->clone_hook_count; ++i) {
        if (reg->clone_hooks[i] == cb) return false;
    }

    if (reg->clone_hook_count >= MAX_CLONE_HOOKS) {
        Log(LOG_LVL_ERROR, "EntitySystem_SubscribeOnCloned: Hook capacity exceeded (%u)", MAX_CLONE_HOOKS);
        assert(false && "EntitySystem_SubscribeOnCloned: Hook capacity exceeded");
        return false;
    }

    reg->clone_hooks[reg->clone_hook_count] = cb;
    reg->clone_hook_count++;
    return true;
}

bool EntitySystem_UnsubscribeOnCloned(EntityRegistry* reg, OnEntityClonedCallback cb) {
    assert(reg != NULL && "reg is NULL");
    assert(cb != NULL && "Unsubscribe callback cannot be NULL!");
    assert(!reg->is_dispatching_clone_hooks && "Cannot unsubscribe during dispatch loop");

    if (reg == NULL) return false;
    if (cb == NULL) return false;

    for (uint8_t i = 0; i < reg->clone_hook_count; ++i) {
        if (reg->clone_hooks[i] != cb) continue;

        for (uint8_t j = i; j + 1 < reg->clone_hook_count; ++j) {
            reg->clone_hooks[j] = reg->clone_hooks[j + 1];
        }

        reg->clone_hook_count--;
        reg->clone_hooks[reg->clone_hook_count] = NULL;
        return true;
    }

    return false;
}

void EntitySystem_ClearCloneHooks(EntityRegistry* reg) {
    assert(reg != NULL && "reg is NULL");
    assert(!reg->is_dispatching_clone_hooks && "Cannot clear hooks during dispatch loop");
    if (reg == NULL) return;

    for (uint8_t i = 0; i < MAX_CLONE_HOOKS; ++i) {
        reg->clone_hooks[i] = NULL;
    }

    reg->clone_hook_count = 0;
    reg->is_dispatching_clone_hooks = false;
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
                const Entity dst = entity;
                const Entity src = cmd->entityClone.prototype;
                const creVec2 spawnPos = cmd->entityClone.position;

                if (dst.id == src.id) break;
                // 2a) dst guards
                if (dst.id >= MAX_ENTITIES) break;

                if (reg->state_flags[dst.id] & FLAG_ACTIVE) {
                    assert(false && "CMD_ENTITY_CLONE: dst slot already active (double-clone)");
                    break;
                }

                if (reg->generations[dst.id] != dst.generation) {
                    EntityManager_ReturnReservedSlot(reg, dst);
                    break;
                }

                // 2c) aliasing guard
                assert(dst.id != src.id && "CMD_ENTITY_CLONE: dst and src alias");

                // 2b) src guards
                if (src.id >= MAX_ENTITIES) {
                    EntityManager_ReturnReservedSlot(reg, dst);
                    break;
                }

                if (!(reg->state_flags[src.id] & FLAG_ACTIVE)) {
                    EntityManager_ReturnReservedSlot(reg, dst);
                    break;
                }

                if (reg->generations[src.id] != src.generation) {
                    EntityManager_ReturnReservedSlot(reg, dst);
                    break;
                }

                // 3) SoA copy (definitions/state only; exclude generations and runtime cursors)
                reg->pos_x[dst.id] = reg->pos_x[src.id];
                reg->pos_y[dst.id] = reg->pos_y[src.id];
                reg->size_w[dst.id] = reg->size_w[src.id];
                reg->size_h[dst.id] = reg->size_h[src.id];
                reg->rotation[dst.id] = reg->rotation[src.id];

                reg->component_masks[dst.id] = reg->component_masks[src.id];
                reg->state_flags[dst.id] = reg->state_flags[src.id] & ~CLONE_FLAGS_SCRUB_MASK;
                reg->types[dst.id] = reg->types[src.id];

                reg->render_layer[dst.id] = reg->render_layer[src.id];
                reg->batch_ids[dst.id] = reg->batch_ids[src.id];
                reg->sprite_ids[dst.id] = reg->sprite_ids[src.id];
                reg->colors[dst.id] = reg->colors[src.id];
                reg->pivot_x[dst.id] = reg->pivot_x[src.id];
                reg->pivot_y[dst.id] = reg->pivot_y[src.id];

                reg->material_id[dst.id] = reg->material_id[src.id];
                reg->drag[dst.id] = reg->drag[src.id];
                reg->inv_mass[dst.id] = reg->inv_mass[src.id];
                reg->gravity_scale[dst.id] = reg->gravity_scale[src.id];

                reg->anim_speeds[dst.id] = reg->anim_speeds[src.id];
                reg->anim_ids[dst.id] = reg->anim_ids[src.id];
                reg->anim_base_durations[dst.id] = reg->anim_base_durations[src.id];
                reg->anim_frame_counts[dst.id] = reg->anim_frame_counts[src.id];
                reg->anim_start_sprites[dst.id] = reg->anim_start_sprites[src.id];
                reg->anim_loops[dst.id] = reg->anim_loops[src.id];

                // 4a) position override
                reg->pos_x[dst.id] = spawnPos.x;
                reg->pos_y[dst.id] = spawnPos.y;

                // 4b) animation cursor reset
                reg->anim_timers[dst.id] = 0.0f;
                reg->anim_frames[dst.id] = 0;
                reg->anim_finished[dst.id] = false;

                // 4c) velocity reset
                reg->vel_x[dst.id] = 0.0f;
                reg->vel_y[dst.id] = 0.0f;

                // 5) awakening
                reg->state_flags[dst.id] |= FLAG_ACTIVE;

                if (dst.id >= reg->max_used_bound) {
                    reg->max_used_bound = (uint32_t)(dst.id + 1);
                }

                reg->active_count++;
                assert(reg->active_count <= MAX_ENTITIES);

                reg->is_dispatching_clone_hooks = true;
#ifndef NDEBUG
                bus->debug_forbidden_domain = CMD_DOMAIN_ENTITY;
#endif
                const uint8_t hook_count = reg->clone_hook_count;
                for (uint8_t i = 0; i < hook_count; ++i) {
                    OnEntityClonedCallback hook = reg->clone_hooks[i];
                    if (hook == NULL) continue;
                    hook(reg, bus, src, dst);
                }
#ifndef NDEBUG
                bus->debug_forbidden_domain = 0;
#endif
                reg->is_dispatching_clone_hooks = false;

                break;
            }
            case CMD_ENTITY_DESTROY: {

                break;
            }
            case CMD_ENTITY_ADD_COMPONENT: {

                break;
            }
            case CMD_ENTITY_REMOVE_COMPONENT: {
                break;
            }
            case CMD_ENTITY_SET_PIVOT: {

                break;
            }

            case CMD_ENTITY_SET_TYPE: {

                break;
            }
            case CMD_ENTITY_SET_FLAGS: {

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
    EntitySystem_ProcessCommands(reg,bus);
}