#include "menu.h"
#include "raylib.h"
#include "engine/scene/cre_sceneManager.h" // For switch scene,
#include "game_scenes.h"
#include "engine/core/cre_logger.h"
#include "engine/platform/cre_input.h"
#include "engine/core/cre_config.h"
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
    int titleWidth = MeasureText(GAME_TITLE, FONT_SIZE_TITLE);
    DrawText(GAME_TITLE, (SCREEN_WIDTH - titleWidth)/2, SCREEN_HEIGHT/2-80, FONT_SIZE_TITLE, RED);

    int startWidth = MeasureText(MENU_START_TEXT, FONT_SIZE_TITLE);
    DrawText(MENU_START_TEXT, (SCREEN_WIDTH - startWidth)/2, SCREEN_HEIGHT/2, FONT_SIZE_TITLE, BLACK);

    int quitWidth = MeasureText(MENU_TO_QUIT, FONT_SIZE_TITLE);
    DrawText(MENU_TO_QUIT, (SCREEN_WIDTH - quitWidth)/2, SCREEN_HEIGHT/2+80, FONT_SIZE_TITLE, BLACK);
}

void Menu_Unload(EntityRegistry* reg, CommandBus* bus) {
    Log(LOG_LVL_INFO,"Scene: Menu Unloaded.");
}

