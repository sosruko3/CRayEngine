#include "game_scenes.h"
#include "game_over.h"
#include "game.h"
#include "engine/core/logger.h"
#include "menu.h"

Scene Game_GetScene(int stateID) {
    Scene scene = { 0 };

    switch (stateID) {
        case GAME_STATE_MENU:
        scene.Init = Menu_Init;
        scene.Update = Menu_Update;
        scene.Draw = Menu_Draw;
        scene.Unload = Menu_Unload;
        break;

        case GAME_STATE_PLAYING:
        scene.Init = Game_Init;
        scene.Update = Game_Update;
        scene.Draw = Game_Draw;
        scene.Unload = Game_Shutdown;
        break;

        case GAME_STATE_GAMEOVER:
        scene.Init   = GameOver_Init;
        scene.Update = GameOver_Update;
        scene.Draw   = GameOver_Draw;
        scene.Unload = GameOver_Unload;
        break;

        default:
        Log(LOG_LVL_WARNING,"Game Scene failed to load.\n");
        break;

    }
    return scene;
 }