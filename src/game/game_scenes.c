#include "game_over.h"
#include "engine/core/game_types.h"
#include "engine/core/scene_manager.h"
#include "game.h"
#include "menu.h"

Scene Game_GetScene(GameState State) {
    Scene scene = { 0 };

    switch (State) {
        case GAME_STATE_PLAYING:
        scene.Init = Game_Init;
        scene.Update = Game_Update;
        scene.Draw = Game_Draw;
        scene.Unload = Game_Shutdown;
        break;

        case GAME_STATE_MENU:
        scene.Init = Menu_Init;
        scene.Update = Menu_Update;
        scene.Draw = Menu_Draw;
        scene.Unload = Menu_Unload;
        break;

        case GAME_STATE_GAMEOVER:
        scene.Init   = GameOver_Init;
        scene.Update = GameOver_Update;
        scene.Draw   = GameOver_Draw;
        scene.Unload = GameOver_Unload;
        break;

        default:
        break;

    }
    return scene;
 }