#ifndef CRE_AUDIOAPI_H
#define CRE_AUDIOAPI_H

#include "engine/core/cre_types.h"
#include "engine/systems/audio/cre_audioSystem.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct CommandBus CommandBus;

AudioID audioAPI_AllocateSound(void);
void audioAPI_GroupInit(CommandBus *restrict bus, uint8_t groupID);
void audioAPI_SetMasterVolume(CommandBus *restrict bus, float volume);
void audioAPI_SetListenerPosition(CommandBus *restrict bus, creVec2 position);

void audioAPI_GroupSetVolume(CommandBus *restrict bus, uint8_t groupID, float volume);
void audioAPI_GroupSetPitch(CommandBus *restrict bus, uint8_t groupID, float pitch);
void audioAPI_GroupSetPan(CommandBus *restrict bus, uint8_t groupID, float pan);

void audioAPI_PlayOneShot(CommandBus *restrict bus, AudioGroupID groupID, AudioSourceID sourceID);

void audioAPI_SoundLoad(CommandBus *restrict bus, AudioID id, AudioSourceID sourceID, 
    AudioGroupID groupID, AudioUsageType usageType);

void audioAPI_SoundUnload(CommandBus *restrict bus, AudioID id);
void audioAPI_SoundPlay(CommandBus *restrict bus, AudioID id);
void audioAPI_SoundPause(CommandBus *restrict bus, AudioID id);
void audioAPI_SoundStop(CommandBus *restrict bus, AudioID id);

// Use between 0.0f and 1.0f for volume value.
void audioAPI_SoundSetVolume(CommandBus *restrict bus, AudioID id, float volume);
// Default pitch value = 1.0f, 2.0f is double speed(high) , 0.5f is half-speed(deep)
void audioAPI_SoundSetPitch(CommandBus *restrict bus, AudioID id, float pitch);
// Default pan value is 0.0f(center) , -1.0f is fully left , 1.0f is fully right side. 
void audioAPI_SoundSetPan(CommandBus *restrict bus, AudioID id, float pan);
void audioAPI_SoundSetLooping(CommandBus *restrict bus, AudioID id, bool looping);
void audioAPI_SoundSetSpatialization(CommandBus *restrict bus, AudioID id, bool enabled);
void audioAPI_SoundSetPosition(CommandBus *restrict bus, AudioID id, creVec2 position);
void audioAPI_SoundSetAttenuation(CommandBus *restrict bus, AudioID id, float minDistance, float maxDistance);

#endif
