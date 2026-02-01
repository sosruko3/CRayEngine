#include "menu.h"
#include "raylib.h"
#include "engine/core/scene_manager.h" // For switch scene,
#include "game_scenes.h"
#include "engine/core/logger.h"
#include "engine/core/ui_helper.h"
#include "engine/core/input.h"
#include "engine/core/config.h"
#include "game_config.h"

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
    DrawTextCentered(GAME_TITLE,SCREEN_HEIGHT/2-80,FONT_SIZE_TITLE,RED);

    DrawTextCentered(MENU_START_TEXT,SCREEN_HEIGHT/2,FONT_SIZE_TITLE,BLACK);
    DrawTextCentered(MENU_TO_QUIT,SCREEN_HEIGHT/2+80,FONT_SIZE_TITLE,BLACK);
}

void Menu_Unload(EntityRegistry* reg, CommandBus* bus) {
    Log(LOG_LVL_INFO,"Scene: Menu Unloaded.");
}

