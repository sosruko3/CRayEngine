#ifndef GAME_SCENES_H
#define GAME_SCENES_H

#include "engine/core/scenes.h"

typedef enum {
    GAME_STATE_MENU,
    GAME_STATE_PLAYING,
    GAME_STATE_GAMEOVER
} GameState;

// Connect game with the engine
Scene Game_GetScene(int stateID);

#endif