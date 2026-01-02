#include "menu.h"
#include "raylib.h"
#include "../engine/core/scene_manager.h" // For switch scene
#include "engine/core/game_types.h" // Needed for STATES
#include "../engine/core/logger.h"
#include "../engine/core/ui_helper.h"
#include "../engine/core/config.h"
#include "game_config.h"

void Menu_Init(void) {
    Log(LOG_LVL_INFO,"Scene: Menu Initialized");
}

void Menu_Update(void) {
    // Future lobby part, if i plan on doing
    if (IsKeyPressed(KEY_ENTER)) {
        SceneManager_ChangeScene(GAME_STATE_PLAYING);
    }
}

void Menu_Draw(void) {
    DrawTextCentered(GAME_TITLE,SCREEN_HEIGHT/2-80,FONT_SIZE_TITLE,RED);

    DrawTextCentered(MENU_START_TEXT,SCREEN_HEIGHT/2,FONT_SIZE_TITLE,BLACK);
    DrawTextCentered(MENU_TO_QUIT,SCREEN_HEIGHT/2+80,FONT_SIZE_TITLE,BLACK);
}

void Menu_Unload(void) {
    Log(LOG_LVL_INFO,"Scene: Menu Unloaded.");
}

