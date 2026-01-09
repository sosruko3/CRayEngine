#include "game.h"
#include "entities/food.h"
#include "entities/snake.h"
#include "engine/core/config.h"
#include "engine/core/ui_helper.h"
#include "raylib.h"
#include "../game_config.h"
#include "engine/core/scene_manager.h"
#include "engine/core/scene_types.h"
#include "engine/core/entity_manager.h"
#include "engine/core/logger.h"
#include "entity_types.h"
#include "game_systems.h"
#include <stdio.h> // For sprintf

// Global variables
int finalScore = 0;


// Helper function
 static void ResetGameplay(void);

void Game_Init(void) {
    EntityManager_Reset();
    finalScore = 0;
    ResetGameplay();
}



void Game_Update(void) {

    float dt = GetFrameTime();
    
    System_HandleDebugInput();

    System_UpdateLogic(dt);
    EntityManager_Update(dt);



    // Snake logic
    UpdateSnake();
    UpdateFood();

    Vector2 head = GetSnakeHeadPosition();
    Vector2 foodPos = GetFoodPosition();

    // Check Food and Snake collision
    if ((int)head.x == (int)foodPos.x && (int)head.y == (int)foodPos.y) 
    {
        GrowSnake();
        SpawnFood(GRID_WIDTH,GRID_HEIGHT); // Remove magic numbers later on
        finalScore += SCORE_PER_FOOD;
    }
         
    // Check Game over Collision
    if (CheckSnakeCollision(GRID_WIDTH,GRID_HEIGHT))
    {
        SceneManager_ChangeScene(GAME_STATE_GAMEOVER);
    }
}
void Game_Draw(void) 
{
    ClearBackground(DARKGRAY);

    // DEBUG: Show FPS
    DrawFPS(10, 10);
    
    DrawSnake();
    DrawFood();
    
    System_DrawEntities();

    // For ScoreHUD
    char scoreText[20];
    sprintf(scoreText,"Score: %d", finalScore);
    DrawText(scoreText,SCREEN_WIDTH * 0.02f,SCREEN_HEIGHT * 0.02f,FONT_SIZE_SCORE,RAYWHITE);
}

void Game_Shutdown(void) {
    // Cleanup if needed
}


static void ResetGameplay(void) {
    EntityManager_Reset();
    InitSnake(GRID_WIDTH/2,GRID_HEIGHT/2);
    InitFood();
    finalScore = 0;

    Entity player = EntityManager_Create(TYPE_PLAYER,(Vector2){100,200});
    // UPDATE HERE,

    EntityData* pData = EntityManager_Get(player);
    if (pData) {
        pData->color = GREEN;
        pData->radius = 20.0f;
        pData->velocity = (Vector2){50,0};
    }
}


