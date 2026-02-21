#include "game_over.h"
#include "raylib.h"
#include "engine/scene/cre_sceneManager.h"
#include "game_scenes.h"
#include "engine/platform/cre_input.h"
#include "game_config.h"
#include "engine/core/cre_config.h"
#include "engine/systems/camera/cre_cameraSystem.h"
#include "engine/systems/render/cre_rendererCore.h"
#include "engine/systems/render/cre_renderSystem.h"
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
    rendererCore_BeginWorldMode(cameraSystem_GetInternal());

    renderSystem_Draw(reg, bus, cameraSystem_GetCullBounds());
    
    rendererCore_EndWorldMode(); 
    rendererCore_EndWorldRender();

    int titleWidth = MeasureText(GAMEOVER_TITLE_TEXT, FONT_SIZE_TITLE);
    DrawText(GAMEOVER_TITLE_TEXT, (SCREEN_WIDTH - titleWidth)/2, SCREEN_HEIGHT/2-50, FONT_SIZE_TITLE, RED);

    int restartWidth = MeasureText(GAMEOVER_RESTART_TEXT, FONT_SIZE_SUBTITLE);
    DrawText(GAMEOVER_RESTART_TEXT, (SCREEN_WIDTH - restartWidth)/2, SCREEN_HEIGHT/2+50, FONT_SIZE_SUBTITLE, RAYWHITE);
    
}

void GameOver_Unload(EntityRegistry* reg, CommandBus* bus) {
    // Nothing to clean up yet

}