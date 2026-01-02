#include "game_over.h"
#include "raylib.h"
#include "engine/core/scene_manager.h"
#include "engine/core/game_types.h"
#include <stdio.h> // for sprintf
#include "engine/core/ui_helper.h"
#include "game_config.h"
#include "engine/core/config.h"


extern int finalScore;

void GameOver_Init(void) {
    // Optional for game over sound effect or anything
}

void GameOver_Update(void) {
    if (IsKeyPressed(KEY_ENTER)) {
        SceneManager_ChangeScene(GAME_STATE_MENU);
    }
}

void GameOver_Draw(void) {

    ClearBackground(BLACK);
    DrawTextCentered(GAMEOVER_TITLE_TEXT,SCREEN_HEIGHT/2-50,FONT_SIZE_TITLE,RED);

    char scoreText[20];
    sprintf(scoreText,"Score: %d", finalScore);
    DrawText(scoreText,SCREEN_WIDTH * 0.02f,SCREEN_HEIGHT * 0.02f,FONT_SIZE_SCORE,RAYWHITE);


    DrawTextCentered(GAMEOVER_RESTART_TEXT,SCREEN_HEIGHT/2+50,FONT_SIZE_SUBTITLE,RAYWHITE);
    
}

void GameOver_Unload(void) {
    // Nothing to clean up yet

}