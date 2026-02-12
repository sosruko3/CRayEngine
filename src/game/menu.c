#include "menu.h"
#include "raylib.h"
#include "engine/scene/cre_sceneManager.h" // For switch scene,
#include "game_scenes.h"
#include "engine/core/cre_logger.h"
#include "engine/core/cre_uiHelper.h"
#include "engine/platform/cre_input.h"
#include "engine/core/cre_config.h"
#include "game_config.h"
#include "engine/core/cre_types.h"

void Menu_Init(EntityRegistry* reg, CommandBus* bus) {
    Log(LOG_LVL_INFO,"Scene: Menu Initialized");
}

void Menu_Update(EntityRegistry* reg, CommandBus* bus,float dt) {
    // Future lobby part, if i plan on doing
    if (Input_IsPressed(ACTION_CONFIRM)) {
        SceneManager_ChangeScene(GAME_STATE_PLAYING);
    }
}

void Menu_Draw(EntityRegistry* reg, CommandBus* bus) {
    DrawTextCentered(GAME_TITLE,SCREEN_HEIGHT/2-80,FONT_SIZE_TITLE,(creColor){230, 41, 55, 255}); /*RED*/

    DrawTextCentered(MENU_START_TEXT,SCREEN_HEIGHT/2,FONT_SIZE_TITLE,(creColor){0, 0, 0, 255}); /*BLACK*/
    DrawTextCentered(MENU_TO_QUIT,SCREEN_HEIGHT/2+80,FONT_SIZE_TITLE,(creColor){0, 0, 0, 255});
}

void Menu_Unload(EntityRegistry* reg, CommandBus* bus) {
    Log(LOG_LVL_INFO,"Scene: Menu Unloaded.");
}

