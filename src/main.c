#include "engine/core/engine.h"
#include "engine/core/config.h"
#include "game_config.h"
#include "engine/core/scene_manager.h"
#include "../game_scenes.h"
#include "game/game.h"

int main(void) {
    Engine_Init(GAME_TITLE,dirCONFIG);
    SceneManager_Init(Game_GetScene);
    SceneManager_ChangeScene(GAME_STATE_MENU);
    Engine_Run();
    Engine_Shutdown();
    return 0;
}
