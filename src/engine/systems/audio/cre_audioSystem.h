#ifndef CRE_AUDIOSYSTEM_H
#define CRE_AUDIOSYSTEM_H

#include <stdint.h>

typedef struct EntityRegistry EntityRegistry;
typedef struct CommandBus CommandBus;

typedef struct {
  uint16_t index;
  uint16_t gen;
} AudioID;

typedef enum { AUDIO_USAGE_STATIC = 0, AUDIO_USAGE_STREAM = 1 } AudioUsageType;

void audioSystem_Init(void);
void audioSystem_Shutdown(void);
void audioSystem_ProcessCommands(EntityRegistry *reg, CommandBus *bus);
void audioSystem_Update(EntityRegistry *reg, CommandBus *bus);

#endif
