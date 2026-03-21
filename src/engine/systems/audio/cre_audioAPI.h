#ifndef CRE_AUDIOAPI_H
#define CRE_AUDIOAPI_H

#include "engine/core/cre_types.h"
#include "engine/systems/audio/cre_audioSystem.h"
#include <stdbool.h>
#include <stdint.h>

struct CommandBus;

AudioID audioAPI_AllocateSound(void);
void audioAPI_GroupInit(CommandBus &bus, AudioGroupID groupID);
void audioAPI_SetMasterVolume(CommandBus &bus, float volume);
void audioAPI_SetListenerPosition(CommandBus &bus, creVec2 position);

// Use between 0.0f and 1.0f for volume value.
void audioAPI_GroupSetVolume(CommandBus &bus, AudioGroupID groupID,
                             float volume);
// Default pitch value = 1.0f, 2.0f is double speed(high) , 0.5f is
// half-speed(deep)
void audioAPI_GroupSetPitch(CommandBus &bus, AudioGroupID groupID, float pitch);
// Default pan value is 0.0f(center) , -1.0f is fully left , 1.0f is fully right
// side.
void audioAPI_GroupSetPan(CommandBus &bus, AudioGroupID groupID, float pan);

void audioAPI_PlayOneShot(CommandBus &bus, AudioGroupID groupID,
                          AudioSourceID sourceID);

void audioAPI_SoundLoad(CommandBus &bus, AudioID id, AudioSourceID sourceID,
                        AudioGroupID groupID, AudioUsageType usageType);

void audioAPI_SoundUnload(CommandBus &bus, AudioID id);
void audioAPI_SoundPlay(CommandBus &bus, AudioID id);
void audioAPI_SoundPause(CommandBus &bus, AudioID id);
void audioAPI_SoundStop(CommandBus &bus, AudioID id);

// Use between 0.0f and 1.0f for volume value.
void audioAPI_SoundSetVolume(CommandBus &bus, AudioID id, float volume);
// Default pitch value = 1.0f, 2.0f is double speed(high) , 0.5f is
// half-speed(deep)
void audioAPI_SoundSetPitch(CommandBus &bus, AudioID id, float pitch);
// Default pan value is 0.0f(center) , -1.0f is fully left , 1.0f is fully right
// side.
void audioAPI_SoundSetPan(CommandBus &bus, AudioID id, float pan);
void audioAPI_SoundSetLooping(CommandBus &bus, AudioID id, bool looping);
void audioAPI_SoundSetSpatialization(CommandBus &bus, AudioID id, bool enabled);
void audioAPI_SoundSetPosition(CommandBus &bus, AudioID id, creVec2 position);
void audioAPI_SoundSetAttenuation(CommandBus &bus, AudioID id,
                                  float minDistance, float maxDistance);

#endif
