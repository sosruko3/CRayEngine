#include "cre_animationAPI.h"
#include "engine/core/cre_commandBus.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

void animAPI_Play(CommandBus &bus, Entity entity, uint16_t animID,
                  bool forceReset) {

  uint16_t flags = forceReset ? ANIM_FLAG_FORCE_RESET : 0;
  Command cmd = {.type = CMD_ANIM_PLAY,
                 .entity = entity,
                 .anim = {.animID = animID, .flags = flags}};
  CommandBus_Push(bus, cmd);
}

void animAPI_Stop(CommandBus &bus, Entity entity) {
  Command cmd = {
      .type = CMD_ANIM_STOP,
      .entity = entity,
      .u16 = {},
  };
  CommandBus_Push(bus, cmd);
}

void animAPI_Resume(CommandBus &bus, Entity entity) {
  Command cmd = {
      .type = CMD_ANIM_RESUME,
      .entity = entity,
      .u16 = {},
  };
  CommandBus_Push(bus, cmd);
}

void animAPI_Pause(CommandBus &bus, Entity entity) {
  Command cmd = {
      .type = CMD_ANIM_PAUSE,
      .entity = entity,
      .u16 = {},
  };
  CommandBus_Push(bus, cmd);
}

void animAPI_SetSpeed(CommandBus &bus, Entity entity, float speed) {
  Command cmd = {
      .type = CMD_ANIM_SET_SPEED, .entity = entity, .f32 = {.value = speed}};
  CommandBus_Push(bus, cmd);
}

void animAPI_SetFrame(CommandBus &bus, Entity entity, uint16_t frame) {
  Command cmd = {
      .type = CMD_ANIM_SET_FRAME, .entity = entity, .u16 = {.value = frame}};
  CommandBus_Push(bus, cmd);
}

void animAPI_SetLoop(CommandBus &bus, Entity entity, bool loop) {
  Command cmd = {
      .type = CMD_ANIM_SET_LOOP, .entity = entity, .b8 = {.value = loop}};
  CommandBus_Push(bus, cmd);
}
