#ifndef GAME_PROTOTYPES_H
#define GAME_PROTOTYPES_H

typedef struct EntityRegistry EntityRegistry;
#include "engine/core/cre_types.h"


extern Entity g_playerPrototype;
extern Entity g_zombiePrototype;

void Prototypes_Init(EntityRegistry* reg);

#endif