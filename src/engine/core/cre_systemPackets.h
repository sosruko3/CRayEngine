#ifndef CRE_SYSTEMPACKETS_H
#define CRE_SYSTEMPACKETS_H

#include <stdint.h>
struct EntityRegistry;
struct CommandBus;
struct TimeContext;
struct creVec2;
struct CameraComponent;

struct scenePacket {
  EntityRegistry *reg;
  CommandBus *bus;
  float gameDt;
};
struct entityPacket {
  EntityRegistry *reg;
  CommandBus *bus;
};
struct physicsPacket {
  CommandBus *bus;
  float fixedDt;
  uint32_t max_used_bound;
  struct ReadAccess {
    const creVec2 *size;
    const uint64_t *component_masks;
    const uint32_t *generations;
  } read;
  struct WriteAccess {
    creVec2 *pos;
    creVec2 *vel;
    uint64_t *state_flags;
    float *inv_mass;
    float *drag;
    float *gravity_scale;
    uint8_t *material_id;
  } write;
};

struct animPacket {
  CommandBus *bus;
  float dt; // gameDt
  uint32_t max_used_bound;
  struct ReadAccess {
    const uint64_t *component_masks;
    const uint32_t *generations;
  } read;
  struct WriteAccess {
    uint64_t *state_flags;
    uint16_t *sprite_ids;
    float *anim_timers;
    float *anim_speeds;
    uint16_t *anim_ids;
    uint16_t *anim_frames;
    bool *anim_finished;
    float *anim_base_durations;
    uint16_t *anim_frame_counts;
    uint16_t *anim_start_sprites;
    bool *anim_loops;
  } write;
};

struct cameraPacket {
  CommandBus *bus;
  float dt;
  struct ReadAccess {
    const uint64_t *component_masks;
    const uint64_t *state_flags;
    const uint32_t *generations;
    const CameraComponent *cameras;
    uint32_t camera_count;
  } read;
  struct WriteAccess {
    creVec2 *pos;
    CameraComponent *cameras;
  } write;
};
#endif
