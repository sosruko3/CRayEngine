#include "cre_audioAPI.h"
#include "engine/core/cre_commandBus.h"
#include "engine/core/cre_logger.h"
#include "engine/core/cre_types.h"
#include <assert.h>

AudioID audioAPI_AllocateSound(void) { return audioSystem_AllocateID(); }

void audioAPI_GroupInit(CommandBus &bus, AudioGroupID groupID) {
  Command cmd = {
      .type = CMD_AUDIO_GROUP_INIT,
      .entity = ENTITY_INVALID,
      .u8 = {.value = groupID},
  };

  if (!CommandBus_Push(bus, cmd)) {
    Log(LOG_LVL_WARNING, "audioAPI_GroupInit: CommandBus is full!");
  }
}

void audioAPI_SetMasterVolume(CommandBus &bus, float volume) {
  Command cmd = {
      .type = CMD_AUDIO_SET_MASTER_VOLUME,
      .entity = ENTITY_INVALID,
      .f32 = {.value = volume},
  };

  if (!CommandBus_Push(bus, cmd)) {
    Log(LOG_LVL_WARNING, "audioAPI_SetMasterVolume: CommandBus is full!");
  }
}

void audioAPI_SetListenerPosition(CommandBus &bus, creVec2 position) {
  Command cmd = {
      .type = CMD_AUDIO_SET_LISTENER_POSITION,
      .entity = ENTITY_INVALID,
      .vec2 = {.value = position},
  };

  if (!CommandBus_Push(bus, cmd)) {
    Log(LOG_LVL_WARNING, "audioAPI_SetListenerPosition: CommandBus is full!");
  }
}

void audioAPI_GroupSetVolume(CommandBus &bus, AudioGroupID groupID,
                             float volume) {
  Command cmd = {
      .type = CMD_AUDIO_GROUP_SET_VOLUME,
      .entity = ENTITY_INVALID,
      .audiogroup =
          {
              .groupID = groupID,
              .value = volume,
          },
  };

  if (!CommandBus_Push(bus, cmd)) {
    Log(LOG_LVL_WARNING, "audioAPI_GroupSetVolume: CommandBus is full!");
  }
}

void audioAPI_GroupSetPitch(CommandBus &bus, AudioGroupID groupID,
                            float pitch) {
  Command cmd = {
      .type = CMD_AUDIO_GROUP_SET_PITCH,
      .entity = ENTITY_INVALID,
      .audiogroup =
          {
              .groupID = groupID,
              .value = pitch,
          },
  };

  if (!CommandBus_Push(bus, cmd)) {
    Log(LOG_LVL_WARNING, "audioAPI_GroupSetPitch: CommandBus is full!");
  }
}

void audioAPI_GroupSetPan(CommandBus &bus, AudioGroupID groupID, float pan) {
  Command cmd = {
      .type = CMD_AUDIO_GROUP_SET_PAN,
      .entity = ENTITY_INVALID,
      .audiogroup =
          {
              .groupID = groupID,
              .value = pan,
          },
  };

  if (!CommandBus_Push(bus, cmd)) {
    Log(LOG_LVL_WARNING, "audioAPI_GroupSetPan: CommandBus is full!");
  }
}

void audioAPI_PlayOneShot(CommandBus &bus, AudioGroupID groupID,
                          AudioSourceID sourceID) {
  Command cmd = {
      .type = CMD_AUDIO_PLAY_ONESHOT,
      .entity = ENTITY_INVALID,
      .audioshot =
          {
              .sourceid = sourceID,
              .groupid = groupID,
          },
  };

  if (!CommandBus_Push(bus, cmd)) {
    Log(LOG_LVL_WARNING, "audioAPI_PlayOneShot: CommandBus is full!");
  }
}

void audioAPI_SoundLoad(CommandBus &bus, AudioID id, AudioSourceID sourceID,
                        AudioGroupID groupID, AudioUsageType usageType) {
  Command cmd = {
      .type = CMD_AUDIO_SOUND_LOAD,
      .entity = ENTITY_INVALID,
      .audioload =
          {
              .id = id,
              .sourceID = sourceID,
              .groupID = groupID,
              .usageType = usageType,
          },
  };

  if (!CommandBus_Push(bus, cmd)) {
    Log(LOG_LVL_WARNING, "audioAPI_SoundLoad: CommandBus is full!");
  }
}

void audioAPI_SoundUnload(CommandBus &bus, AudioID id) {
  Command cmd = {
      .type = CMD_AUDIO_SOUND_UNLOAD,
      .entity = ENTITY_INVALID,
      .audioid = {.id = id},
  };

  if (!CommandBus_Push(bus, cmd)) {
    Log(LOG_LVL_WARNING, "audioAPI_SoundUnload: CommandBus is full!");
  }
}

void audioAPI_SoundPlay(CommandBus &bus, AudioID id) {
  Command cmd = {
      .type = CMD_AUDIO_SOUND_PLAY,
      .entity = ENTITY_INVALID,
      .audioid = {.id = id},
  };

  if (!CommandBus_Push(bus, cmd)) {
    Log(LOG_LVL_WARNING, "audioAPI_SoundPlay: CommandBus is full!");
  }
}

void audioAPI_SoundPause(CommandBus &bus, AudioID id) {
  Command cmd = {
      .type = CMD_AUDIO_SOUND_PAUSE,
      .entity = ENTITY_INVALID,
      .audioid = {.id = id},
  };

  if (!CommandBus_Push(bus, cmd)) {
    Log(LOG_LVL_WARNING, "audioAPI_SoundPause: CommandBus is full!");
  }
}

void audioAPI_SoundStop(CommandBus &bus, AudioID id) {
  Command cmd = {
      .type = CMD_AUDIO_SOUND_STOP,
      .entity = ENTITY_INVALID,
      .audioid = {.id = id},
  };

  if (!CommandBus_Push(bus, cmd)) {
    Log(LOG_LVL_WARNING, "audioAPI_SoundStop: CommandBus is full!");
  }
}

void audioAPI_SoundSetVolume(CommandBus &bus, AudioID id, float volume) {
  Command cmd = {
      .type = CMD_AUDIO_SOUND_SET_VOLUME,
      .entity = ENTITY_INVALID,
      .audiof32 =
          {
              .id = id,
              .value = volume,
          },
  };

  if (!CommandBus_Push(bus, cmd)) {
    Log(LOG_LVL_WARNING, "audioAPI_SoundSetVolume: CommandBus is full!");
  }
}

void audioAPI_SoundSetPitch(CommandBus &bus, AudioID id, float pitch) {
  Command cmd = {
      .type = CMD_AUDIO_SOUND_SET_PITCH,
      .entity = ENTITY_INVALID,
      .audiof32 =
          {
              .id = id,
              .value = pitch,
          },
  };

  if (!CommandBus_Push(bus, cmd)) {
    Log(LOG_LVL_WARNING, "audioAPI_SoundSetPitch: CommandBus is full!");
  }
}

void audioAPI_SoundSetPan(CommandBus &bus, AudioID id, float pan) {
  Command cmd = {
      .type = CMD_AUDIO_SOUND_SET_PAN,
      .entity = ENTITY_INVALID,
      .audiof32 =
          {
              .id = id,
              .value = pan,
          },
  };

  if (!CommandBus_Push(bus, cmd)) {
    Log(LOG_LVL_WARNING, "audioAPI_SoundSetPan: CommandBus is full!");
  }
}

void audioAPI_SoundSetLooping(CommandBus &bus, AudioID id, bool looping) {
  Command cmd = {
      .type = CMD_AUDIO_SOUND_SET_LOOPING,
      .entity = ENTITY_INVALID,
      .audiob8 =
          {
              .id = id,
              .value = looping,
          },
  };

  if (!CommandBus_Push(bus, cmd)) {
    Log(LOG_LVL_WARNING, "audioAPI_SoundSetLooping: CommandBus is full!");
  }
}

void audioAPI_SoundSetSpatialization(CommandBus &bus, AudioID id,
                                     bool enabled) {
  Command cmd = {
      .type = CMD_AUDIO_SOUND_SET_SPATIALIZATION,
      .entity = ENTITY_INVALID,
      .audiob8 =
          {
              .id = id,
              .value = enabled,
          },
  };

  if (!CommandBus_Push(bus, cmd)) {
    Log(LOG_LVL_WARNING,
        "audioAPI_SoundSetSpatialization: CommandBus is full!");
  }
}

void audioAPI_SoundSetPosition(CommandBus &bus, AudioID id, creVec2 position) {
  Command cmd = {
      .type = CMD_AUDIO_SOUND_SET_POSITION,
      .entity = ENTITY_INVALID,
      .audiovec2 =
          {
              .id = id,
              .value = position,
          },
  };

  if (!CommandBus_Push(bus, cmd)) {
    Log(LOG_LVL_WARNING, "audioAPI_SoundSetPosition: CommandBus is full!");
  }
}

void audioAPI_SoundSetAttenuation(CommandBus &bus, AudioID id,
                                  float minDistance, float maxDistance) {
  Command cmd = {
      .type = CMD_AUDIO_SOUND_SET_ATTENUATION,
      .entity = ENTITY_INVALID,
      .audiovec2 =
          {
              .id = id,
              .value = {.x = minDistance, .y = maxDistance},
          },
  };

  if (!CommandBus_Push(bus, cmd)) {
    Log(LOG_LVL_WARNING, "audioAPI_SoundSetAttenuation: CommandBus is full!");
  }
}
