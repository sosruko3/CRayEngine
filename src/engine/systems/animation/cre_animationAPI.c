#include "cre_animationAPI.h"
#include "engine/core/cre_commandBus.h"
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

void animAPI_Play(CommandBus* restrict bus, Entity entity,uint16_t animID, bool forceReset) {

    uint16_t flags = forceReset ? ANIM_FLAG_FORCE_RESET : 0;
    assert(bus != NULL);
    Command cmd = {
        .type = CMD_ANIM_PLAY,
        .entity = entity,
        .anim = {
            .animID = animID, 
            .flags = flags
        }   
    };
    CommandBus_Push(bus,cmd);
}

void animAPI_Stop(CommandBus* restrict bus, Entity entity) {
    assert(bus != NULL);
    Command cmd = {
        .type = CMD_ANIM_STOP,
        .entity = entity,
    };
    CommandBus_Push(bus,cmd);
}

void animAPI_Resume(CommandBus* restrict bus, Entity entity) {
    assert(bus != NULL);
    Command cmd = {
        .type = CMD_ANIM_RESUME,
        .entity = entity,
    };
    CommandBus_Push(bus,cmd);
}

void animAPI_Pause(CommandBus* restrict bus, Entity entity) {
    assert(bus != NULL);
    Command cmd = {
        .type = CMD_ANIM_PAUSE,
        .entity = entity,
    };
    CommandBus_Push(bus,cmd);
}

void animAPI_SetSpeed(CommandBus* restrict bus, Entity entity,float speed) {
    assert(bus != NULL);
    Command cmd = {
        .type = CMD_ANIM_SET_SPEED,
        .entity = entity,
        .f32 = { .value = speed }
    };
    CommandBus_Push(bus,cmd);
}

void animAPI_SetFrame(CommandBus* restrict bus, Entity entity, uint16_t frame) {
    assert(bus != NULL);
    Command cmd = {
        .type = CMD_ANIM_SET_FRAME,
        .entity = entity,
        .u16 = { .value = frame }
    };
    CommandBus_Push(bus,cmd);
}

void animAPI_SetLoop(CommandBus* restrict bus, Entity entity, bool loop) {
    assert(bus != NULL);
    Command cmd = {
        .type = CMD_ANIM_SET_LOOP,
        .entity = entity,
        .b8 = {.value = loop }
    };
    CommandBus_Push(bus,cmd);
}
