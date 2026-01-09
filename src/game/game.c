#include "game.h"
#include "entities/food.h"
#include "entities/snake.h"
#include "engine/core/config.h"
#include "engine/core/ui_helper.h"
#include "raylib.h"
#include "../game_config.h"
#include "engine/core/scene_manager.h"
#include "engine/core/game_types.h"
#include "engine/core/entity_manager.h"
#include "engine/core/logger.h"
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

// PRESS 'Z' TO SPAWN THE HORDE
if (IsKeyPressed(KEY_Z)) {
    for (int i = 0; i < 1000; i++) {
        // 1. Random Position
        int x = GetRandomValue(0, SCREEN_WIDTH);
        int y = GetRandomValue(0, SCREEN_HEIGHT);
        
        Entity e = EntityManager_Create(1, (Vector2){x, y});
        
        // 2. Random Speed & Color
        EntityData* data = EntityManager_Get(e);
        if (data) {
            data->velocity.x = GetRandomValue(-200, 200); // Fast random movement
            data->velocity.y = GetRandomValue(-200, 200);
            
            // Random Color
            data->color = (Color){ 
                GetRandomValue(50, 255), 
                GetRandomValue(50, 255), 
                GetRandomValue(50, 255), 
                255 
            };
            data->radius = 3.0f; // Tiny particles
        }
    }
    Log(LOG_LVL_INFO,"Spawned 1000 entities!\n");
}
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
    

    // For ScoreHUD
    char scoreText[20];
    sprintf(scoreText,"Score: %d", finalScore);
    DrawText(scoreText,SCREEN_WIDTH * 0.02f,SCREEN_HEIGHT * 0.02f,FONT_SIZE_SCORE,RAYWHITE);
}

void Game_Shutdown(void) {
    // Cleanup if needed
}


static void ResetGameplay(void) {
    InitSnake(GRID_WIDTH/2,GRID_HEIGHT/2);
    InitFood();
    finalScore = 0;

    Entity player = EntityManager_Create(0,(Vector2){100,200});

    EntityData* pData = EntityManager_Get(player);
    if (pData) {
        pData->color = GREEN;
        pData->radius = 20.0f;
        pData->velocity = (Vector2){50,0};
    }
}
