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
#include "engine/physics/physics_system.h"
#include "engine/core/asset_manager.h"
#include "engine/core/animation.h"
#include "engine/core/cre_renderer.h"

// Global variables
int finalScore = 0;
static double physicsTime = 0.0; // FOR DEBUG


// Helper function
 static void ResetGameplay(void);

void Game_Init(void) {
    Asset_Init();
    EntityManager_Reset();
    PhysicsSystem_Init();
    finalScore = 0;
    ResetGameplay();
}
void Game_Update(void) {
    float dt = GetFrameTime();
    System_HandleDebugInput();
    SystemTestSpawn();
    System_UpdateLogic(dt);

    double startTime = GetTime(); // FOR DEBUG
    PhysicsSystem_Update(dt);
    double endTime = GetTime(); // FOR DEBUG
    physicsTime = (endTime - startTime) * 1000.0; // FOR DEBUG

    AnimationSystem_Update(dt);

    // Snake logic
    //UpdateSnake();
    //UpdateFood();
    Vector2 head = GetSnakeHeadPosition();
    Vector2 foodPos = GetFoodPosition();

    // Check Food and Snake collision
    if ((int)head.x == (int)foodPos.x && (int)head.y == (int)foodPos.y) {
        GrowSnake();
        SpawnFood(GRID_WIDTH,GRID_HEIGHT); // Remove magic numbers later on
        finalScore += SCORE_PER_FOOD;
    }
         
    // Check Game over Collision
    if (CheckSnakeCollision(GRID_WIDTH,GRID_HEIGHT)) SceneManager_ChangeScene(GAME_STATE_GAMEOVER);
}
void Game_Draw(void) {
    //DrawSnake();
    //DrawFood();
    System_DrawEntities();

    // For ScoreHUD
    char scoreText[20];
    sprintf(scoreText,"Score: %d", finalScore);
    float uiPadding = creRenderer_GetVirtualWidth() * 0.02f;
    DrawText(scoreText, uiPadding, 20, 30, RAYWHITE);
    
    DrawFPS(10, 10); // FOR DEBUG
    int activeCount = GetActiveEntityCount(); // FOR DEBUG
    DrawText(TextFormat("Physics time: %.04f ms | Entities: %d",physicsTime, activeCount),20,20,20,RED); // FOR DEBUG
}
void Game_Shutdown(void) {
    // Cleanup if needed
}
static void ResetGameplay(void) {
    EntityManager_Reset();
    InitSnake(GRID_WIDTH/2,GRID_HEIGHT/2);
    InitFood();
    finalScore = 0;
    // this is for test, change location later on
    SpawnPlayer();
}


