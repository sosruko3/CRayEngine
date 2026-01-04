#include "scene_manager.h"
#include "logger.h"
#include "stddef.h" // for NULL

#include "../../game/game_scenes.h"

// Internal state
static Scene currentScene = { 0 };
static GameState activeState = -1; // -1 = No state

void SceneManager_Init(void) {
    Log(LOG_LVL_INFO,"Scene Manager Initialized.");
}

void SceneManager_Update(void) {
    if (currentScene.Update) {
            currentScene.Update();
    }
}
void SceneManager_Draw(void) {
    if (currentScene.Draw) {
        currentScene.Draw();
    }
}

void SceneManager_Shutdown(void) {
    if (currentScene.Unload) {
        currentScene.Unload();
    }
}

void SceneManager_ChangeScene(GameState nextState) {
    Log(LOG_LVL_INFO, "Changing Scene...");

    // Unload the old scene
    if (currentScene.Unload) {
        currentScene.Unload();
    }

    currentScene = Game_GetScene(nextState);
    activeState = nextState;

    if (currentScene.Init) {
        currentScene.Init();
    }
}