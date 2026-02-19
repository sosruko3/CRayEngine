#include "game_over.h"
#include "raylib.h"
#include "engine/scene/cre_sceneManager.h"
#include "game_scenes.h"
#include "engine/platform/cre_input.h"
#include "engine/core/cre_uiHelper.h"
#include "game_config.h"
#include "engine/core/cre_config.h"
#include "engine/core/cre_types.h"
#include <stdio.h> // for sprintf


void GameOver_Init(EntityRegistry* reg, CommandBus* bus) {
    // Optional for game over sound effect or anything
}

void GameOver_Update(EntityRegistry* reg, CommandBus* bus,float dt) {
    if (Input_IsPressed(ACTION_CONFIRM)) {
        SceneManager_ChangeScene(GAME_STATE_MENU);
    }
}

void GameOver_Draw(EntityRegistry* reg, CommandBus* bus) {

    ClearBackground(BLACK);
    DrawTextCentered(GAMEOVER_TITLE_TEXT,SCREEN_HEIGHT/2-50,FONT_SIZE_TITLE,(creColor){230, 41, 55, 255}); /*RED*/

    DrawTextCentered(GAMEOVER_RESTART_TEXT,SCREEN_HEIGHT/2+50,FONT_SIZE_SUBTITLE,(creColor){245, 245, 245, 255}); /*RAYWHITE*/
    
}

void GameOver_Unload(EntityRegistry* reg, CommandBus* bus) {
    // Nothing to clean up yet

}