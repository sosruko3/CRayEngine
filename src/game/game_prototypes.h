#ifndef GAME_PROTOTYPES_H
#define GAME_PROTOTYPES_H

#include "engine/core/cre_types.h"
struct EntityRegistry;

extern Entity g_playerPrototype;
extern Entity g_zombiePrototype;

void Prototypes_Init(EntityRegistry &reg);

#endif
