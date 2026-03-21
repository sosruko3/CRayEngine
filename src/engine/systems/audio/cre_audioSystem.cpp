#include "cre_audioSystem.h"
#include "engine/core/cre_commandBus.h"
#include "engine/core/cre_logger.h"
#include "engine/ecs/cre_entityRegistry.h"
#include "external/miniaudio/miniaudio.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

/* Section 1: Includes, Constants, and Static Variables */
constexpr uint16_t AUDIO_SOUND_POOL_CAPACITY = 256;

static ma_engine s_audioEngine;
static ma_resource_manager s_resourceManager;
static bool s_audioInitialized = false;
static ma_sound s_soundPool[AUDIO_SOUND_POOL_CAPACITY];
static ma_sound_group s_groups[AUDIO_GROUP_COUNT];
static bool s_soundLoaded[AUDIO_SOUND_POOL_CAPACITY];
static bool s_groupInitialized[AUDIO_GROUP_COUNT];
static uint16_t s_soundGeneration[AUDIO_SOUND_POOL_CAPACITY];
static uint16_t s_freeIndices[AUDIO_SOUND_POOL_CAPACITY];
static int16_t s_freeTop = -1;

// This is temporary, will be adding automated version later on.
static const char *s_sourcePaths[AUDIO_SOURCE_COUNT] = {
    "assets/sounds/test_sfx.ogg",  // AUDIO_SOURCE_TEST_SFX
    "assets/sounds/test_bgm.ogg"}; // AUDIO_SOURCE_TEST_BGM

// Matching our flags with miniaudio's flags.
// Don't try to use two of them at once right now, will fix that later on.
static const ma_uint32 s_usageFlags[] = {
    MA_SOUND_FLAG_DECODE, // AUDIO_USAGE_STATIC
    MA_SOUND_FLAG_STREAM, // AUDIO_USAGE_STREAM
    MA_SOUND_FLAG_ASYNC,  // AUDIO_USAGE_ASYNC
};

/* Forward declarations for internal wrappers used by public init/update flow.
 */
static bool audio_ValidateUsage(AudioUsageType usage);
static bool audio_ValidateGroupID(AudioGroupID groupID);
static bool audio_ValidateSoundID(AudioID id, uint16_t *outIndex,
                                  bool requireLoaded);

static void audio_GroupInit(AudioGroupID groupID);
static void audio_SetMasterVolume(float vol);
static void audio_ListenerSetPosition(float x, float y);
static void audio_GroupSetVolume(AudioGroupID groupID, float vol);
static void audio_GroupSetPitch(AudioGroupID groupID, float pitch);
static void audio_GroupSetPan(AudioGroupID groupID, float pan);
static void audio_PlayOneShot(AudioSourceID sourceID, AudioGroupID groupID);
static void audio_SoundLoad(AudioID id, AudioSourceID sourceID,
                            AudioGroupID groupID, AudioUsageType usage);
static void audio_SoundUnload(AudioID id);
static void audio_SoundPlay(AudioID id);
static void audio_SoundPause(AudioID id);
static void audio_SoundStop(AudioID id);
static void audio_SoundSetVolume(AudioID id, float vol);
static void audio_SoundSetPitch(AudioID id, float pitch);
static void audio_SoundSetPan(AudioID id, float pan);
static void audio_SoundSetLooping(AudioID id, bool loop);
static void audio_SoundSetSpatialization(AudioID id, bool enable);
static void audio_SoundSetPosition(AudioID id, float x, float y);
static void audio_SoundSetAttenuation(AudioID id, float min, float max);

/* Section 2: Public Engine Functions  */
void audioSystem_Init(void) {
  if (s_audioInitialized)
    return;

  ma_resource_manager_config rmConfig = ma_resource_manager_config_init();
  rmConfig.decodedFormat = ma_format_f32;
  rmConfig.decodedChannels = 0;
  rmConfig.decodedSampleRate = 48000;

  ma_result result = ma_resource_manager_init(&rmConfig, &s_resourceManager);
  if (result != MA_SUCCESS) {
    Log(LOG_LVL_ERROR, "[AUDIO] Resource Manager init failed (err=%d)",
        static_cast<int32_t>(result));
    return;
  }

  ma_engine_config engineConfig = ma_engine_config_init();
  engineConfig.pResourceManager = &s_resourceManager;

  result = ma_engine_init(&engineConfig, &s_audioEngine);
  if (result != MA_SUCCESS) {
    Log(LOG_LVL_WARNING, "[AUDIO] ma_engine_init failed (err=%d)",
        static_cast<int32_t>(result));
    ma_resource_manager_uninit(&s_resourceManager);
    return;
  }

  s_audioInitialized = true;
  audio_GroupInit(AUDIO_GROUP_MASTER);
  audio_GroupInit(AUDIO_GROUP_BGM);
  audio_GroupInit(AUDIO_GROUP_SFX);
  audio_GroupInit(AUDIO_GROUP_UI);
  audio_GroupInit(AUDIO_GROUP_ENV);

  for (uint16_t i = 0; i < AUDIO_SOUND_POOL_CAPACITY; i++) {
    s_freeIndices[++s_freeTop] = i;
    s_soundGeneration[i] = 0;
  }
}

void audioSystem_Update(EntityRegistry &reg, CommandBus &bus) {
  audioSystem_ProcessCommands(reg, bus);
}

void audioSystem_ProcessCommands(EntityRegistry &reg, CommandBus &bus) {
  (void)reg;

  CommandIterator iter = CommandBus_GetIterator(bus);
  const Command *cmd;

  while (CommandBus_Next(bus, &iter, &cmd)) {
    if ((cmd->type & CMD_DOMAIN_MASK) != CMD_DOMAIN_AUDIO) {
      continue;
    }

    switch (cmd->type) {
    case CMD_AUDIO_GROUP_INIT:
      audio_GroupInit(static_cast<AudioGroupID>(cmd->u8.value));
      break;
    case CMD_AUDIO_SET_MASTER_VOLUME:
      audio_SetMasterVolume(cmd->f32.value);
      break;
    case CMD_AUDIO_SET_LISTENER_POSITION:
      audio_ListenerSetPosition(cmd->vec2.value.x, cmd->vec2.value.y);
      break;
    case CMD_AUDIO_GROUP_SET_VOLUME:
      audio_GroupSetVolume(cmd->audiogroup.groupID, cmd->audiogroup.value);
      break;
    case CMD_AUDIO_GROUP_SET_PITCH:
      audio_GroupSetPitch(cmd->audiogroup.groupID, cmd->audiogroup.value);
      break;
    case CMD_AUDIO_GROUP_SET_PAN:
      audio_GroupSetPan(cmd->audiogroup.groupID, cmd->audiogroup.value);
      break;
    case CMD_AUDIO_PLAY_ONESHOT:
      audio_PlayOneShot(cmd->audioshot.sourceid, cmd->audioshot.groupid);
      break;
    case CMD_AUDIO_SOUND_LOAD:
      audio_SoundLoad(cmd->audioload.id, cmd->audioload.sourceID,
                      cmd->audioload.groupID, cmd->audioload.usageType);
      break;
    case CMD_AUDIO_SOUND_UNLOAD:
      audio_SoundUnload(cmd->audioid.id);
      break;
    case CMD_AUDIO_SOUND_PLAY:
      audio_SoundPlay(cmd->audioid.id);
      break;
    case CMD_AUDIO_SOUND_PAUSE:
      audio_SoundPause(cmd->audioid.id);
      break;
    case CMD_AUDIO_SOUND_STOP:
      audio_SoundStop(cmd->audioid.id);
      break;
    case CMD_AUDIO_SOUND_SET_VOLUME:
      audio_SoundSetVolume(cmd->audiof32.id, cmd->audiof32.value);
      break;
    case CMD_AUDIO_SOUND_SET_PITCH:
      audio_SoundSetPitch(cmd->audiof32.id, cmd->audiof32.value);
      break;
    case CMD_AUDIO_SOUND_SET_PAN:
      audio_SoundSetPan(cmd->audiof32.id, cmd->audiof32.value);
      break;
    case CMD_AUDIO_SOUND_SET_LOOPING:
      audio_SoundSetLooping(cmd->audiob8.id, cmd->audiob8.value);
      break;
    case CMD_AUDIO_SOUND_SET_SPATIALIZATION:
      audio_SoundSetSpatialization(cmd->audiob8.id, cmd->audiob8.value);
      break;
    case CMD_AUDIO_SOUND_SET_POSITION:
      audio_SoundSetPosition(cmd->audiovec2.id, cmd->audiovec2.value.x,
                             cmd->audiovec2.value.y);
      break;
    case CMD_AUDIO_SOUND_SET_ATTENUATION:
      audio_SoundSetAttenuation(cmd->audiovec2.id, cmd->audiovec2.value.x,
                                cmd->audiovec2.value.y);
      break;
    default:
      break;
    }
  }
}

void audioSystem_Shutdown(void) {
  assert(s_audioInitialized && "Audio system is not initalized.");

  for (uint16_t i = 0; i < AUDIO_SOUND_POOL_CAPACITY; ++i) {
    if (s_soundLoaded[i]) {
      ma_sound_uninit(&s_soundPool[i]);
      s_soundLoaded[i] = false;
      ++s_soundGeneration[i];
    }
  }

  for (uint8_t i = 0; i < AUDIO_GROUP_COUNT; ++i) {
    if (s_groupInitialized[i]) {
      ma_sound_group_uninit(&s_groups[i]);
      s_groupInitialized[i] = false;
    }
  }

  ma_engine_uninit(&s_audioEngine);
  ma_resource_manager_uninit(&s_resourceManager);
  s_audioInitialized = false;
  s_freeTop = -1;
}

/* Section 3: Internal Guardrail Functions */
static bool audio_ValidateUsage(AudioUsageType usage) {
  if ((uint32_t)usage >= AUDIO_USAGE_COUNT) {
    Log(LOG_LVL_WARNING, "[AUDIO] Invalid usage enum: %u", (unsigned)usage);
    return false;
  }

  return true;
}

static bool audio_ValidateGroupID(AudioGroupID groupID) {
  if (groupID >= AUDIO_GROUP_COUNT) {
    Log(LOG_LVL_WARNING, "[AUDIO] Group ID out of bounds: %u",
        (unsigned)groupID);
    return false;
  }

  return true;
}

static bool audio_ValidateSoundID(AudioID id, uint16_t *outIndex,
                                  bool requireLoaded) {
  const uint16_t index = id.index;
  const uint16_t generation = id.gen;

  if (index >= AUDIO_SOUND_POOL_CAPACITY) {
    Log(LOG_LVL_WARNING, "[AUDIO] Sound ID index out of bounds: %u",
        (unsigned)index);
    return false;
  }

  if (generation != s_soundGeneration[index]) {
    Log(LOG_LVL_WARNING,
        "[AUDIO] Stale sound handle. idx=%u gen=%u expected=%u",
        (unsigned)index, (unsigned)generation,
        (unsigned)s_soundGeneration[index]);
    return false;
  }

  if (requireLoaded && !s_soundLoaded[index]) {
    Log(LOG_LVL_WARNING, "[AUDIO] Sound slot is not loaded. idx=%u",
        (unsigned)index);
    return false;
  }

  *outIndex = index;
  return true;
}

AudioID audioSystem_AllocateID(void) {
  if (s_freeTop < 0) {
    Log(LOG_LVL_ERROR, "[AUDIO] Pool exhausted! No free slots.");
    return AudioID{.index = 0, .gen = 0xFFFF};
  }
  uint16_t index = s_freeIndices[s_freeTop--];
  return AudioID{.index = index, .gen = s_soundGeneration[index]};
}

/* Section 4: Static Internal Wrapper API */
static void audio_SetMasterVolume(float vol) {
  assert(s_audioInitialized && "Audio system is not initalized.");

  ma_engine_set_volume(&s_audioEngine, vol);
}

static void audio_ListenerSetPosition(float x, float y) {
  assert(s_audioInitialized && "Audio system is not initalized.");

  ma_engine_listener_set_position(&s_audioEngine, 0, x, y, 0.0f);
}

static void audio_GroupInit(AudioGroupID groupID) {
  assert(s_audioInitialized && "Audio system is not initalized.");

  if (!audio_ValidateGroupID(groupID)) {
    return;
  }

  if (s_groupInitialized[groupID]) {
    Log(LOG_LVL_WARNING, "[AUDIO] Group already initialized: %u",
        (unsigned)groupID);
    return;
  }

  const ma_result result =
      ma_sound_group_init(&s_audioEngine, 0, NULL, &s_groups[groupID]);
  if (result != MA_SUCCESS) {
    Log(LOG_LVL_WARNING, "[AUDIO] Failed to init group %u (err=%d)",
        (unsigned)groupID, (int)result);
    return;
  }

  s_groupInitialized[groupID] = true;
}

static void audio_GroupSetVolume(AudioGroupID groupID, float vol) {
  assert(s_audioInitialized && "Audio system is not initalized.");

  if (!audio_ValidateGroupID(groupID) || !s_groupInitialized[groupID]) {
    Log(LOG_LVL_WARNING, "[AUDIO] Group volume set on uninitialized group: %u",
        (unsigned)groupID);
    return;
  }

  ma_sound_group_set_volume(&s_groups[groupID], vol);
}

static void audio_GroupSetPitch(AudioGroupID groupID, float pitch) {
  assert(s_audioInitialized && "Audio system is not initalized.");

  if (!audio_ValidateGroupID(groupID) || !s_groupInitialized[groupID]) {
    Log(LOG_LVL_WARNING, "[AUDIO] Group pitch set on uninitialized group: %u",
        (unsigned)groupID);
    return;
  }

  ma_sound_group_set_pitch(&s_groups[groupID], pitch);
}

static void audio_GroupSetPan(AudioGroupID groupID, float pan) {
  assert(s_audioInitialized && "Audio system is not initalized.");

  if (!audio_ValidateGroupID(groupID) || !s_groupInitialized[groupID]) {
    Log(LOG_LVL_WARNING, "[AUDIO] Group pan set on uninitialized group: %u",
        (unsigned)groupID);
    return;
  }

  ma_sound_group_set_pan(&s_groups[groupID], pan);
}

static void audio_PlayOneShot(AudioSourceID sourceID, AudioGroupID groupID) {
  assert(s_audioInitialized && "Audio system is not initalized.");
  assert(s_groupInitialized[groupID] &&
         "Group must be initialized before use!");

  if (sourceID >= AUDIO_SOURCE_COUNT) {
    Log(LOG_LVL_WARNING, "[AUDIO] Invalid source ID: %u", (unsigned)sourceID);
    return;
  }

  const char *filepath = s_sourcePaths[sourceID];
  const ma_result result =
      ma_engine_play_sound(&s_audioEngine, filepath, &s_groups[groupID]);

  if (result != MA_SUCCESS) {
    Log(LOG_LVL_WARNING, "[AUDIO] One-shot play failed: %s (err=%d)", filepath,
        (int)result);
  }
}

static void audio_SoundLoad(AudioID id, AudioSourceID sourceID,
                            AudioGroupID groupID, AudioUsageType usage) {
  uint16_t slot = 0;
  assert(s_audioInitialized && "Audio system is not initalized.");
  assert(s_groupInitialized[groupID] &&
         "Group must be initialized before use!");

  if (sourceID >= AUDIO_SOURCE_COUNT) {
    Log(LOG_LVL_WARNING, "[AUDIO] Invalid source ID: %u", (unsigned)sourceID);
    return;
  }

  if (!audio_ValidateUsage(usage) || !audio_ValidateSoundID(id, &slot, false)) {
    return;
  }

  if (s_soundLoaded[slot]) {
    Log(LOG_LVL_WARNING, "[AUDIO] Reloading loaded sound slot: %u",
        (unsigned)slot);
    ma_sound_uninit(&s_soundPool[slot]);
    s_soundLoaded[slot] = false;
  }

  const char *filepath = s_sourcePaths[sourceID];
  const ma_uint32 flags = s_usageFlags[(uint32_t)usage];
  const ma_result result =
      ma_sound_init_from_file(&s_audioEngine, filepath, flags,
                              &s_groups[groupID], NULL, &s_soundPool[slot]);

  if (result != MA_SUCCESS) {
    Log(LOG_LVL_WARNING, "[AUDIO] Failed to load sound idx=%u path=%s err=%d",
        (unsigned)slot, filepath, (int)result);
    return;
  }

  s_soundLoaded[slot] = true;
}

static void audio_SoundUnload(AudioID id) {
  uint16_t slot = 0;
  assert(s_audioInitialized && "Audio system is not initalized.");

  if (!audio_ValidateSoundID(id, &slot, true)) {
    return;
  }

  ma_sound_uninit(&s_soundPool[slot]);
  s_soundLoaded[slot] = false;
  ++s_soundGeneration[slot];
  s_freeIndices[++s_freeTop] = slot;
}

static void audio_SoundPlay(AudioID id) {
  uint16_t slot = 0;
  assert(s_audioInitialized && "Audio system is not initalized.");

  if (!audio_ValidateSoundID(id, &slot, true)) {
    return;
  }

  const ma_result result = ma_sound_start(&s_soundPool[slot]);
  if (result != MA_SUCCESS) {
    Log(LOG_LVL_WARNING, "[AUDIO] Failed to start sound idx=%u err=%d",
        (unsigned)slot, (int)result);
  }
}

static void audio_SoundPause(AudioID id) {
  uint16_t slot = 0;
  assert(s_audioInitialized && "Audio system is not initalized.");

  if (!audio_ValidateSoundID(id, &slot, true)) {
    return;
  }

  const ma_result result = ma_sound_stop(&s_soundPool[slot]);
  if (result != MA_SUCCESS) {
    Log(LOG_LVL_WARNING, "[AUDIO] Failed to pause sound idx=%u err=%d",
        (unsigned)slot, (int)result);
  }
}

static void audio_SoundStop(AudioID id) {
  uint16_t slot = 0;
  assert(s_audioInitialized && "Audio system is not initalized.");

  if (!audio_ValidateSoundID(id, &slot, true)) {
    return;
  }

  ma_result result = ma_sound_stop(&s_soundPool[slot]);
  if (result != MA_SUCCESS) {
    Log(LOG_LVL_WARNING, "[AUDIO] Failed to stop sound idx=%u err=%d",
        (unsigned)slot, (int)result);
    return;
  }

  result = ma_sound_seek_to_pcm_frame(&s_soundPool[slot], 0);
  if (result != MA_SUCCESS) {
    Log(LOG_LVL_WARNING, "[AUDIO] Failed to rewind sound idx=%u err=%d",
        (unsigned)slot, (int)result);
  }
}

static void audio_SoundSetVolume(AudioID id, float vol) {
  uint16_t slot = 0;
  assert(s_audioInitialized && "Audio system is not initalized.");

  if (!audio_ValidateSoundID(id, &slot, true)) {
    return;
  }

  ma_sound_set_volume(&s_soundPool[slot], vol);
}

static void audio_SoundSetPitch(AudioID id, float pitch) {
  uint16_t slot = 0;
  assert(s_audioInitialized && "Audio system is not initalized.");

  if (!audio_ValidateSoundID(id, &slot, true)) {
    return;
  }

  ma_sound_set_pitch(&s_soundPool[slot], pitch);
}

static void audio_SoundSetPan(AudioID id, float pan) {
  uint16_t slot = 0;
  assert(s_audioInitialized && "Audio system is not initalized.");

  if (!audio_ValidateSoundID(id, &slot, true)) {
    return;
  }

  ma_sound_set_pan(&s_soundPool[slot], pan);
}

static void audio_SoundSetLooping(AudioID id, bool loop) {
  uint16_t slot = 0;
  assert(s_audioInitialized && "Audio system is not initalized.");

  if (!audio_ValidateSoundID(id, &slot, true)) {
    return;
  }

  ma_sound_set_looping(&s_soundPool[slot], loop ? MA_TRUE : MA_FALSE);
}

static void audio_SoundSetSpatialization(AudioID id, bool enable) {
  uint16_t slot = 0;
  assert(s_audioInitialized && "Audio system is not initalized.");

  if (!audio_ValidateSoundID(id, &slot, true)) {
    return;
  }

  ma_sound_set_spatialization_enabled(&s_soundPool[slot],
                                      enable ? MA_TRUE : MA_FALSE);
}

static void audio_SoundSetPosition(AudioID id, float x, float y) {
  uint16_t slot = 0;
  assert(s_audioInitialized && "Audio system is not initalized.");

  if (!audio_ValidateSoundID(id, &slot, true)) {
    return;
  }

  ma_sound_set_position(&s_soundPool[slot], x, y, 0.0f);
}

static void audio_SoundSetAttenuation(AudioID id, float min, float max) {
  uint16_t slot = 0;
  assert(s_audioInitialized && "Audio system is not initalized.");

  if (!audio_ValidateSoundID(id, &slot, true)) {
    return;
  }

  if (min < 0.0f || max < 0.0f || min > max) {
    Log(LOG_LVL_WARNING, "[AUDIO] Invalid attenuation min=%.3f max=%.3f",
        (double)min, (double)max);
    return;
  }

  ma_sound_set_min_distance(&s_soundPool[slot], min);
  ma_sound_set_max_distance(&s_soundPool[slot], max);
}
