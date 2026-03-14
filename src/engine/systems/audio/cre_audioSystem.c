#include "cre_audioSystem.h"
#include "engine/core/cre_commandBus.h"
#include "engine/core/cre_logger.h"
#include "engine/ecs/cre_entityRegistry.h"
#include "external/miniaudio/miniaudio.h"
#include <assert.h>
#include <stdbool.h>

/* Section 1: Includes, Constants, and Static Variables */
enum {
  AUDIO_SOUND_POOL_CAPACITY = 256,
  AUDIO_GROUP_CAPACITY = 16,
  AUDIO_DEFAULT_GROUP_SLOT = 0,
  AUDIO_USAGE_COUNT = 2
};

static ma_engine s_audioEngine;
static bool s_audioInitialized = false;
static ma_sound s_soundPool[AUDIO_SOUND_POOL_CAPACITY];
static ma_sound_group s_groups[AUDIO_GROUP_CAPACITY];
static bool s_soundLoaded[AUDIO_SOUND_POOL_CAPACITY];
static bool s_groupInitialized[AUDIO_GROUP_CAPACITY];
static uint16_t s_soundGeneration[AUDIO_SOUND_POOL_CAPACITY];

static const ma_uint32 s_usageFlags[] = {
    [AUDIO_USAGE_STATIC] = MA_SOUND_FLAG_DECODE,
    [AUDIO_USAGE_STREAM] = MA_SOUND_FLAG_STREAM};

/* Forward declarations for internal wrappers used by public init/update flow.
 */
static bool audio_ValidateUsage(AudioUsageType usage);
static bool audio_ValidateGroupID(uint8_t groupID);
static bool audio_ValidateSoundID(AudioID id, uint16_t *outIndex,
                                  bool requireLoaded);

static void audio_GroupInit(uint8_t groupID);
static void audio_SetMasterVolume(float vol);
static void audio_ListenerSetPosition(float x, float y);
static void audio_GroupSetVolume(uint8_t groupID, float vol);
static void audio_GroupSetPitch(uint8_t groupID, float pitch);
static void audio_GroupSetPan(uint8_t groupID, float pan);
static void audio_PlayOneShot(const char *filepath);
static void audio_SoundLoad(AudioID id, const char *filepath,
                            AudioUsageType usage);
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

/* Section 2: Public Engine Functions (audioSystem_Init, Update,
 * ProcessCommands, Shutdown) */
void audioSystem_Init(void) {
  if (!s_audioInitialized) {
    const ma_result result = ma_engine_init(NULL, &s_audioEngine);
    if (result != MA_SUCCESS) {
      Log(LOG_LVL_WARNING, "[AUDIO] ma_engine_init failed (err=%d)",
          (int)result);
      return;
    }

    s_audioInitialized = true;
    audio_GroupInit(AUDIO_DEFAULT_GROUP_SLOT);
  }
}

void audioSystem_Update(EntityRegistry *reg, CommandBus *bus) {
  audioSystem_ProcessCommands(reg, bus);
}

void audioSystem_ProcessCommands(EntityRegistry *reg, CommandBus *bus) {
  assert(reg && "reg is NULL");
  assert(bus && "bus is NULL");

  CommandIterator iter = CommandBus_GetIterator(bus);
  const Command *cmd;

  while (CommandBus_Next(bus, &iter, &cmd)) {
    if ((cmd->type & CMD_DOMAIN_MASK) != CMD_DOMAIN_AUDIO) {
      continue;
    }

    switch (cmd->type) {
    default:
      break;
    }
  }
}

void audioSystem_Shutdown(void) {
  if (!s_audioInitialized) {
    Log(LOG_LVL_WARNING, "[AUDIO] audioSystem_Shutdown called before init");
    return;
  }

  for (uint16_t i = 0; i < AUDIO_SOUND_POOL_CAPACITY; ++i) {
    if (s_soundLoaded[i]) {
      ma_sound_uninit(&s_soundPool[i]);
      s_soundLoaded[i] = false;
      ++s_soundGeneration[i];
    }
  }

  for (uint8_t i = 0; i < AUDIO_GROUP_CAPACITY; ++i) {
    if (s_groupInitialized[i]) {
      ma_sound_group_uninit(&s_groups[i]);
      s_groupInitialized[i] = false;
    }
  }

  ma_engine_uninit(&s_audioEngine);
  s_audioInitialized = false;
}

/* Section 3: Internal Guardrail Functions */
static bool audio_ValidateUsage(AudioUsageType usage) {
  if ((uint32_t)usage >= AUDIO_USAGE_COUNT) {
    Log(LOG_LVL_WARNING, "[AUDIO] Invalid usage enum: %u", (unsigned)usage);
    return false;
  }

  return true;
}

static bool audio_ValidateGroupID(uint8_t groupID) {
  if (groupID >= AUDIO_GROUP_CAPACITY) {
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

/* Section 4: Static Internal Wrapper API */
static void audio_SetMasterVolume(float vol) {
  if (!s_audioInitialized) {
    Log(LOG_LVL_WARNING, "[AUDIO] audio_SetMasterVolume called before init");
    return;
  }

  ma_engine_set_volume(&s_audioEngine, vol);
}

static void audio_ListenerSetPosition(float x, float y) {
  if (!s_audioInitialized) {
    Log(LOG_LVL_WARNING,
        "[AUDIO] audio_ListenerSetPosition called before init");
    return;
  }

  ma_engine_listener_set_position(&s_audioEngine, 0, x, y, 0.0f);
}

static void audio_GroupInit(uint8_t groupID) {
  if (!s_audioInitialized) {
    Log(LOG_LVL_WARNING, "[AUDIO] audio_GroupInit called before init");
    return;
  }

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

static void audio_GroupSetVolume(uint8_t groupID, float vol) {
  if (!s_audioInitialized) {
    Log(LOG_LVL_WARNING, "[AUDIO] audio_GroupSetVolume called before init");
    return;
  }

  if (!audio_ValidateGroupID(groupID) || !s_groupInitialized[groupID]) {
    Log(LOG_LVL_WARNING, "[AUDIO] Group volume set on uninitialized group: %u",
        (unsigned)groupID);
    return;
  }

  ma_sound_group_set_volume(&s_groups[groupID], vol);
}

static void audio_GroupSetPitch(uint8_t groupID, float pitch) {
  if (!s_audioInitialized) {
    Log(LOG_LVL_WARNING, "[AUDIO] audio_GroupSetPitch called before init");
    return;
  }

  if (!audio_ValidateGroupID(groupID) || !s_groupInitialized[groupID]) {
    Log(LOG_LVL_WARNING, "[AUDIO] Group pitch set on uninitialized group: %u",
        (unsigned)groupID);
    return;
  }

  ma_sound_group_set_pitch(&s_groups[groupID], pitch);
}

static void audio_GroupSetPan(uint8_t groupID, float pan) {
  if (!s_audioInitialized) {
    Log(LOG_LVL_WARNING, "[AUDIO] audio_GroupSetPan called before init");
    return;
  }

  if (!audio_ValidateGroupID(groupID) || !s_groupInitialized[groupID]) {
    Log(LOG_LVL_WARNING, "[AUDIO] Group pan set on uninitialized group: %u",
        (unsigned)groupID);
    return;
  }

  ma_sound_group_set_pan(&s_groups[groupID], pan);
}

static void audio_PlayOneShot(const char *filepath) {
  if (!s_audioInitialized) {
    Log(LOG_LVL_WARNING, "[AUDIO] audio_PlayOneShot called before init");
    return;
  }

  if (filepath == NULL) {
    Log(LOG_LVL_WARNING, "[AUDIO] audio_PlayOneShot filepath is NULL");
    return;
  }

  const ma_result result = ma_engine_play_sound(&s_audioEngine, filepath, NULL);
  if (result != MA_SUCCESS) {
    Log(LOG_LVL_WARNING, "[AUDIO] One-shot play failed: %s (err=%d)", filepath,
        (int)result);
  }
}

static void audio_SoundLoad(AudioID id, const char *filepath,
                            AudioUsageType usage) {
  uint16_t slot = 0;
  if (!s_audioInitialized) {
    Log(LOG_LVL_WARNING, "[AUDIO] audio_SoundLoad called before init");
    return;
  }

  if (filepath == NULL) {
    Log(LOG_LVL_WARNING, "[AUDIO] audio_SoundLoad filepath is NULL");
    return;
  }

  if (!audio_ValidateUsage(usage) || !audio_ValidateSoundID(id, &slot, false)) {
    return;
  }

  if (!s_groupInitialized[AUDIO_DEFAULT_GROUP_SLOT]) {
    Log(LOG_LVL_WARNING, "[AUDIO] Default group slot is not initialized");
    return;
  }

  if (s_soundLoaded[slot]) {
    Log(LOG_LVL_WARNING, "[AUDIO] Reloading loaded sound slot: %u",
        (unsigned)slot);
    ma_sound_uninit(&s_soundPool[slot]);
    s_soundLoaded[slot] = false;
  }

  const ma_uint32 flags = s_usageFlags[(uint32_t)usage];
  const ma_result result = ma_sound_init_from_file(
      &s_audioEngine, filepath, flags, &s_groups[AUDIO_DEFAULT_GROUP_SLOT],
      NULL, &s_soundPool[slot]);

  if (result != MA_SUCCESS) {
    Log(LOG_LVL_WARNING, "[AUDIO] Failed to load sound idx=%u path=%s err=%d",
        (unsigned)slot, filepath, (int)result);
    return;
  }

  s_soundLoaded[slot] = true;
}

static void audio_SoundUnload(AudioID id) {
  uint16_t slot = 0;
  if (!s_audioInitialized) {
    Log(LOG_LVL_WARNING, "[AUDIO] audio_SoundUnload called before init");
    return;
  }

  if (!audio_ValidateSoundID(id, &slot, true)) {
    return;
  }

  ma_sound_uninit(&s_soundPool[slot]);
  s_soundLoaded[slot] = false;
  ++s_soundGeneration[slot];
}

static void audio_SoundPlay(AudioID id) {
  uint16_t slot = 0;
  if (!s_audioInitialized) {
    Log(LOG_LVL_WARNING, "[AUDIO] audio_SoundPlay called before init");
    return;
  }

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
  if (!s_audioInitialized) {
    Log(LOG_LVL_WARNING, "[AUDIO] audio_SoundPause called before init");
    return;
  }

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
  if (!s_audioInitialized) {
    Log(LOG_LVL_WARNING, "[AUDIO] audio_SoundStop called before init");
    return;
  }

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
  if (!s_audioInitialized) {
    Log(LOG_LVL_WARNING, "[AUDIO] audio_SoundSetVolume called before init");
    return;
  }

  if (!audio_ValidateSoundID(id, &slot, true)) {
    return;
  }

  ma_sound_set_volume(&s_soundPool[slot], vol);
}

static void audio_SoundSetPitch(AudioID id, float pitch) {
  uint16_t slot = 0;
  if (!s_audioInitialized) {
    Log(LOG_LVL_WARNING, "[AUDIO] audio_SoundSetPitch called before init");
    return;
  }

  if (!audio_ValidateSoundID(id, &slot, true)) {
    return;
  }

  ma_sound_set_pitch(&s_soundPool[slot], pitch);
}

static void audio_SoundSetPan(AudioID id, float pan) {
  uint16_t slot = 0;
  if (!s_audioInitialized) {
    Log(LOG_LVL_WARNING, "[AUDIO] audio_SoundSetPan called before init");
    return;
  }

  if (!audio_ValidateSoundID(id, &slot, true)) {
    return;
  }

  ma_sound_set_pan(&s_soundPool[slot], pan);
}

static void audio_SoundSetLooping(AudioID id, bool loop) {
  uint16_t slot = 0;
  if (!s_audioInitialized) {
    Log(LOG_LVL_WARNING, "[AUDIO] audio_SoundSetLooping called before init");
    return;
  }

  if (!audio_ValidateSoundID(id, &slot, true)) {
    return;
  }

  ma_sound_set_looping(&s_soundPool[slot], loop ? MA_TRUE : MA_FALSE);
}

static void audio_SoundSetSpatialization(AudioID id, bool enable) {
  uint16_t slot = 0;
  if (!s_audioInitialized) {
    Log(LOG_LVL_WARNING,
        "[AUDIO] audio_SoundSetSpatialization called before init");
    return;
  }

  if (!audio_ValidateSoundID(id, &slot, true)) {
    return;
  }

  ma_sound_set_spatialization_enabled(&s_soundPool[slot],
                                      enable ? MA_TRUE : MA_FALSE);
}

static void audio_SoundSetPosition(AudioID id, float x, float y) {
  uint16_t slot = 0;
  if (!s_audioInitialized) {
    Log(LOG_LVL_WARNING, "[AUDIO] audio_SoundSetPosition called before init");
    return;
  }

  if (!audio_ValidateSoundID(id, &slot, true)) {
    return;
  }

  ma_sound_set_position(&s_soundPool[slot], x, y, 0.0f);
}

static void audio_SoundSetAttenuation(AudioID id, float min, float max) {
  uint16_t slot = 0;
  if (!s_audioInitialized) {
    Log(LOG_LVL_WARNING,
        "[AUDIO] audio_SoundSetAttenuation called before init");
    return;
  }

  if (!audio_ValidateSoundID(id, &slot, true)) {
    return;
  }

  if (min < 0.0f || max < 0.0f || min > max) {
    Log(LOG_LVL_WARNING, "[AUDIO] Invalid attenuation min=%.3f max=%.3f", min,
        max);
    return;
  }

  ma_sound_set_min_distance(&s_soundPool[slot], min);
  ma_sound_set_max_distance(&s_soundPool[slot], max);
}
