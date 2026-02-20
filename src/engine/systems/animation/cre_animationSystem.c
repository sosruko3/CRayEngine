/**
 * @file cre_animationSystem.c
 * @brief Pure SoA Animation System Implementation (Baked Data Architecture)
 * 
 * The "Baker" pattern: AnimationSystem_Play copies constant animation data
 * from ASSET_ANIMS into registry arrays. The hot loop then reads ONLY from
 * the registry, achieving pure linear memory access with zero lookups.
 */

#include "cre_animationSystem.h"
#include "engine/ecs/cre_entityRegistry.h"
#include "engine/core/cre_commandBus.h"
#include "atlas_data.h"  // Only used in AnimationSystem_Play (the baker)
#include "assert.h"

//============================================================================
// Helper functions
//============================================================================
static bool IsValidAnimEntity(EntityRegistry* reg,Entity entity) {
    uint32_t id = entity.id;
    uint32_t gen = entity.generation;
    if (id >= MAX_ENTITIES) return false;
    if (gen != reg->generations[id]) return false;
    if (!(reg->component_masks[id] & COMP_ANIMATION)) return false;
    if (!(reg->state_flags[id] & FLAG_ACTIVE)) return false;
    return true;
}
// ============================================================================
// Command Processing (Cold Path)
// ============================================================================

void AnimationSystem_ProcessCommands(EntityRegistry* reg, const CommandBus* bus) {
    assert(reg && "reg is NULL");
    assert(bus && "Bus is mandatory!");

    CommandIterator iter = CommandBus_GetIterator(bus);
    const Command* cmd;

    while (CommandBus_Next(bus, &iter, &cmd)) {
        // this part is kinda fragile, fix later on.
        if ((cmd->type & CMD_DOMAIN_MASK) != CMD_DOMAIN_ANIM) continue;
        Entity entity = cmd->entity;
        const uint32_t id = entity.id;
        if (!IsValidAnimEntity(reg,entity)) continue;
        switch (cmd->type) {
            case CMD_ANIM_PLAY: {
                const uint16_t animID = cmd->anim.animID;
                const bool forceReset = (cmd->anim.flags & ANIM_FLAG_FORCE_RESET) != 0;
                AnimationSystem_Play(reg, id, animID, forceReset);
                break;
            }
            case CMD_ANIM_STOP: {
                // Stop animation: mark finished, keep current frame
                reg->anim_finished[id] = true;
                reg->anim_timers[id] = 0.0f;
                reg->anim_frames[id] = 0;
                break;
            }
            case CMD_ANIM_PAUSE: {
                reg->state_flags[id] |= FLAG_ANIM_PAUSED;
                break;
            }
            case CMD_ANIM_RESUME: {
            reg->state_flags[id] &= ~FLAG_ANIM_PAUSED;
                break;
            }
            default:

                break;
        }
    }
}

// ============================================================================
// The Baker - Copies constant data into registry on animation start
// ============================================================================

void AnimationSystem_Play(EntityRegistry* reg, uint32_t entityID, uint16_t animID, bool forceReset) {
    assert(reg && "reg is NULL");
    assert(animID < ANIM_COUNT && "Invalid Animation ID! Check your Anim enum.");

    // Skip if already playing this animation (unless forced)
    if (!forceReset && 
        reg->anim_ids[entityID] == animID && 
        !reg->anim_finished[entityID]) {
        return;
    }

    // === THE BAKING STEP ===
    // Look up the AnimDef ONCE and copy constants into the registry
    const AnimDef* def = &ASSET_ANIMS[animID];

    // Bake constant data (copied from AnimDef)
    reg->anim_base_durations[entityID] = def->defaultSpeed;
    reg->anim_frame_counts[entityID]   = def->frameCount;
    reg->anim_start_sprites[entityID]  = def->startSpriteID;
    reg->anim_loops[entityID]          = def->loop;

    // Reset dynamic state
    reg->anim_ids[entityID]      = animID;
    reg->anim_frames[entityID]   = 0;
    reg->anim_timers[entityID]   = 0.0f;
    reg->anim_finished[entityID] = false;

    // Set initial sprite immediately
    reg->sprite_ids[entityID] = def->startSpriteID;
}

void AnimationSystem_SetSpeed(EntityRegistry* reg, uint32_t entityID, float multiplier) {
    assert(reg && "reg is NULL");
    if (entityID >= MAX_ENTITIES) return;
    reg->anim_speeds[entityID] = multiplier;
}

// ============================================================================
// The Hot Loop - Pure SoA
// ============================================================================

void AnimationSystem_Update(EntityRegistry* reg, float dt) {
    assert(reg && "reg is NULL");

    // Clamp delta time to prevent spiral of death
    if (dt > 0.05f) dt = 0.05f;

    // Cache array pointers for hot loop (12 streams + masks/flags)
    const uint64_t* masks    = reg->component_masks;
    const uint64_t* flags    = reg->state_flags;
    uint32_t max_used_bound  = reg->max_used_bound;
    
    // Dynamic state arrays
    float*    timers         = reg->anim_timers;
    float*    speeds         = reg->anim_speeds;
    uint16_t* frames         = reg->anim_frames;
    bool*     finished       = reg->anim_finished;
    uint16_t* sprites        = reg->sprite_ids;
    
    // Baked constant arrays (read-only in hot loop)
    const float*    base_durations = reg->anim_base_durations;
    const uint16_t* frame_counts   = reg->anim_frame_counts;
    const uint16_t* start_sprites  = reg->anim_start_sprites;
    const bool*     loops          = reg->anim_loops;
    

    const uint64_t required_mask  = COMP_ANIMATION;
    const uint64_t required_flags = FLAG_ACTIVE;
    const uint64_t notrequired_flags = FLAG_ANIM_PAUSED;

    for (uint32_t i = 0; i < max_used_bound; ++i) {
        // If possible, make this part branchless in the future.
        if (!(masks[i] & required_mask)) continue;
        if (!(flags[i] & required_flags)) continue;
        if ( (flags[i] & notrequired_flags)) continue;
 
        // Skip already finished animations
        if (finished[i]) continue;

        // Critical safety: prevent infinite loop on invalid duration
        const float duration = base_durations[i];
        if (duration <= 0.0001f) {
            finished[i] = true;
            continue;
        }

        // Calculate effective delta time with speed multiplier
        float speed = speeds[i];
        if (speed <= 0.0001f) continue;  // Paused (speed = 0)
        
        const float effectiveDt = dt * speed;
        timers[i] += effectiveDt;

        // Advance frames (while loop handles low FPS frame skipping)
        while (timers[i] >= duration) {
            timers[i] -= duration;
            frames[i]++;

            // Handle end of animation
            if (frames[i] >= frame_counts[i]) {
                if (loops[i]) {
                    frames[i] = 0;
                } else {
                    frames[i] = frame_counts[i] - 1;
                    finished[i] = true;
                    break;
                }
            }
        }

        // Optimization: Only write sprite if entity is visible
        if (flags[i] & FLAG_VISIBLE) {
            sprites[i] = start_sprites[i] + frames[i];
        }
    }
}