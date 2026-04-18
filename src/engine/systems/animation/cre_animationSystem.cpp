/**
 * @file cre_animationSystem.c
 * @brief Pure SoA Animation System Implementation (Baked Data Architecture)
 *
 * [NOTE] This system will get cleaning refactor, also directional anim feature.
 */

#include "cre_animationSystem.h"
#include "assert.h"
#include "atlas_data.h"
#include "engine/core/cre_commandBus.h"
#include "engine/core/cre_systemPackets.h"
#include "engine/ecs/cre_entityRegistry.h"
animPacket CreateAnimPacket(EntityRegistry *reg, CommandBus *bus, float dt) {
  animPacket pkt = {
      .bus = bus,
      .dt = dt,
      .max_used_bound = reg->max_used_bound,
      .read = {.component_masks = reg->component_masks,
               .generations = reg->generations},
      .write = {.state_flags = reg->state_flags,
                .sprite_ids = reg->sprite_ids,
                .anim_timers = reg->anim_timers,
                .anim_speeds = reg->anim_speeds,
                .anim_ids = reg->anim_ids,
                .anim_frames = reg->anim_frames,
                .anim_finished = reg->anim_finished,
                .anim_base_durations = reg->anim_base_durations,
                .anim_frame_counts = reg->anim_frame_counts,
                .anim_start_sprites = reg->anim_start_sprites,
                .anim_loops = reg->anim_loops},
  };
  return pkt;
}
// ============================================================================
// Command Processing
// ============================================================================

void AnimationSystem_ProcessCommands(animPacket *packet) {
  // UNPACKING THE PACKET
  CommandBus &bus = *packet->bus;
  const uint64_t *masks = packet->read.component_masks;
  const uint32_t *generations = packet->read.generations;
  uint16_t *anim_ids = packet->write.anim_ids;
  uint64_t *flags = packet->write.state_flags;
  uint16_t *sprite_ids = packet->write.sprite_ids;
  float *anim_timers = packet->write.anim_timers;
  float *anim_speeds = packet->write.anim_speeds;
  uint16_t *anim_frames = packet->write.anim_frames;
  bool *anim_finished = packet->write.anim_finished;
  float *base_durations = packet->write.anim_base_durations;
  uint16_t *frame_counts = packet->write.anim_frame_counts;
  uint16_t *start_sprites = packet->write.anim_start_sprites;
  bool *anim_loops = packet->write.anim_loops;

  CommandIterator iter = CommandBus_GetIterator(*packet->bus);
  const Command *cmd;

  while (CommandBus_Next(bus, &iter, &cmd)) {
    if ((cmd->type & CMD_DOMAIN_MASK) != CMD_DOMAIN_ANIM)
      continue;
    // This assumes every anim commands has entity. Change this if you add some
    // global command.
    if (!EntityRegistry_IsAlive(flags, generations, cmd->entity))
      continue;
    const Entity entity = cmd->entity;
    const uint32_t id = entity.id;
    if (!(masks[id] & COMP_ANIMATION))
      continue;

    switch (cmd->type) {
    case CMD_ANIM_PLAY: {
      const uint16_t animID = cmd->anim.animID;
      const bool forceReset = (cmd->anim.flags & ANIM_FLAG_FORCE_RESET) != 0;
      assert(animID < ANIM_COUNT &&
             "Invalid Animation ID! Check your Anim enum.");
      if (animID >= ANIM_COUNT)
        return;

      // Skip if already playing this animation (unless forced)
      if (!forceReset && anim_ids[id] == animID && !anim_finished[id]) {
        return;
      }

      // === THE BAKING STEP ===
      // Look up the AnimDef ONCE and copy constants into the registry
      const AnimDef *def = &ASSET_ANIMS[animID];

      // Bake constant data (copied from AnimDef)
      base_durations[id] = def->defaultSpeed;
      frame_counts[id] = def->frameCount;
      start_sprites[id] = def->startSpriteID;
      anim_loops[id] = def->loop;

      // Reset dynamic state
      anim_ids[id] = animID;
      anim_frames[id] = 0;
      anim_timers[id] = 0.0f;
      anim_speeds[id] = 1.0f;
      anim_finished[id] = false;
      // Set initial sprite immediately
      sprite_ids[id] = def->startSpriteID;

      break;
    }
    case CMD_ANIM_STOP: {
      anim_finished[id] = true;
      anim_timers[id] = 0.0f;
      anim_frames[id] = 0;
      break;
    }
    case CMD_ANIM_PAUSE: {
      flags[id] |= FLAG_ANIM_PAUSED;
      break;
    }
    case CMD_ANIM_RESUME: {
      flags[id] &= ~FLAG_ANIM_PAUSED;
      break;
    }
    case CMD_ANIM_SET_FRAME: {
      // Set current frame safely and restart frame timer
      uint16_t frame = cmd->u16.value;
      if (frame_counts[id] == 0) {
        anim_frames[id] = 0;
        anim_timers[id] = 0.0f;
        anim_finished[id] = true;
        break;
      }

      if (frame >= frame_counts[id]) {
        frame = frame_counts[id];
        --frame; // frame = frame_count -1U;
      }

      anim_frames[id] = frame;
      anim_timers[id] = 0.0f;
      // Policy: Revive-on-SetFrame so explicit frame seek resumes animation
      // flow.
      anim_finished[id] = false;
      break;
    }
    case CMD_ANIM_SET_SPEED: {
      // Set animation speed multiplier (0 = paused)
      float speed = cmd->f32.value;
      if (speed < 0.0001f) {
        speed = 0.0f;
      }
      anim_speeds[id] = speed;
      break;
    }
    case CMD_ANIM_SET_LOOP: {
      // Override loop flag and revive if loop is enabled
      const bool loop = cmd->b8.value;
      anim_loops[id] = loop;
      if (loop) {
        anim_finished[id] = false;
      }
      break;
    }
    default:
      break;
    }
  }
}

// ============================================================================
// The Hot Loop - Pure SoA
// ============================================================================

void AnimationSystem_Update(animPacket *packet) {
  // UNPACKING THE PACKET
  float dt = packet->dt;
  const uint32_t max_used_bound = packet->max_used_bound;
  const uint64_t *restrict masks = packet->read.component_masks;
  uint64_t *restrict flags = packet->write.state_flags;
  uint16_t *restrict sprite_ids = packet->write.sprite_ids;
  float *restrict anim_timers = packet->write.anim_timers;
  float *restrict anim_speeds = packet->write.anim_speeds;
  uint16_t *restrict anim_frames = packet->write.anim_frames;
  bool *restrict finished = packet->write.anim_finished;
  float *restrict base_durations = packet->write.anim_base_durations;
  uint16_t *restrict frame_counts = packet->write.anim_frame_counts;
  uint16_t *restrict start_sprites = packet->write.anim_start_sprites;
  bool *restrict anim_loops = packet->write.anim_loops;

  // Clamp delta time to prevent spiral of death
  if (dt > 0.05f)
    dt = 0.05f;

  AnimationSystem_ProcessCommands(packet); // HANDLE THIS PART!

  const uint64_t required_mask = COMP_ANIMATION;
  const uint64_t required_flags = FLAG_ACTIVE;
  const uint64_t notrequired_flags = FLAG_ANIM_PAUSED;

  for (uint32_t i = 0; i < max_used_bound; ++i) {
    // If possible, make this part branchless in the future.
    if (!(masks[i] & required_mask))
      continue;
    if (!(flags[i] & required_flags))
      continue;
    if ((flags[i] & notrequired_flags))
      continue;

    // Skip already finished animations
    if (finished[i]) {
      // Draw last frame?
      if (flags[i] & FLAG_VISIBLE)
        sprite_ids[i] = start_sprites[i] + anim_frames[i];
      continue;
    }
    // Critical safety: prevent infinite loop on invalid duration
    const float duration = base_durations[i];
    if (duration <= 0.0001f) {
      finished[i] = true;
      continue;
    }

    // Calculate effective delta time with speed multiplier
    float speed = anim_speeds[i];
    if (speed <= 0.0001f)
      continue; // Paused (speed = 0)

    const float effectiveDt = dt * speed;
    anim_timers[i] += effectiveDt;

    // Advance frames (while loop handles low FPS frame skipping)
    while (anim_timers[i] >= duration) {
      anim_timers[i] -= duration;
      anim_frames[i]++;

      // Handle end of animation
      if (anim_frames[i] >= frame_counts[i]) {
        if (anim_loops[i]) {
          anim_frames[i] = 0;
        } else {
          anim_frames[i] = frame_counts[i] - 1;
          finished[i] = true;
          break;
        }
      }
    }

    // Optimization: Only write sprite if entity is visible
    if (flags[i] & FLAG_VISIBLE) {
      sprite_ids[i] = start_sprites[i] + anim_frames[i];
    }
  }
}
