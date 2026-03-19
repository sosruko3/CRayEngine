#ifndef CRE_AUDIOSYSTEM_H
#define CRE_AUDIOSYSTEM_H

#include "engine/core/cre_types.h"
#include <stdint.h>

struct EntityRegistry;
struct CommandBus;

enum AudioSourceID : uint16_t {
  AUDIO_SOURCE_TEST_SFX = 0,
  AUDIO_SOURCE_TEST_BGM = 1,
  AUDIO_SOURCE_COUNT
};

enum AudioUsageType : uint8_t {
  AUDIO_USAGE_STATIC = 0,
  AUDIO_USAGE_STREAM = 1,
  AUDIO_USAGE_ASYNC = 2,
  AUDIO_USAGE_COUNT
};

enum AudioGroupID : uint8_t {
  AUDIO_GROUP_MASTER = 0,
  AUDIO_GROUP_BGM,
  AUDIO_GROUP_SFX,
  AUDIO_GROUP_UI,
  AUDIO_GROUP_ENV,
  AUDIO_GROUP_COUNT
};

void audioSystem_Init(void);
void audioSystem_Shutdown(void);
void audioSystem_ProcessCommands(EntityRegistry *reg, CommandBus *bus);
void audioSystem_Update(EntityRegistry *reg, CommandBus *bus);
AudioID audioSystem_AllocateID(void);

#endif
