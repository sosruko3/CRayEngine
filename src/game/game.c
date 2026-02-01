#include "game.h"
#include "entities/food.h"
#include "entities/snake.h"
#include "engine/core/config.h"
#include "engine/core/ui_helper.h"
#include "raylib.h"
#include "../game_config.h"
#include "engine/core/scene_manager.h"
#include "game_scenes.h"
#include "engine/core/entity_manager.h"
#include "engine/core/logger.h"
#include "entity_types.h"
#include "controlSystem.h"
#include "debugSystem.h"
#include "engine/core/cre_RenderSystem.h"
#include <stdio.h> // For sprintf
#include "engine/physics/physics_system.h"
#include "engine/core/asset_manager.h"
#include "engine/core/animationSystem.h"
#include "engine/core/cre_RendererCore.h"
#include "engine/core/viewport.h"
#include "engine/core/cre_camera.h"

// Global variables
int finalScore = 0;
static double physicsTime = 0.0; // FOR DEBUG
static EntityRegistry* s_gameReg = NULL; // Store registry pointer for ResetGameplay

// Helper function
 static void ResetGameplay(EntityRegistry* reg, CommandBus* bus);

void Game_Init(EntityRegistry* reg, CommandBus* bus) {
    if (!reg || !bus) {
        Log(LOG_LVL_ERROR, "CRITICAL: Game_Init received NULL pointers!");
        return; // Stop before we crash
    }
    s_gameReg = reg;
    finalScore = 0;
    ResetGameplay(reg, bus);
}
void Game_Update(EntityRegistry* reg, CommandBus* bus,float dt) {
    ControlSystem_UpdateSleepState(reg);
    DebugSystem_HandleInput(reg, bus);
    DebugSystem_SpawnTestEntity(reg, bus);
    ControlSystem_UpdateLogic(reg, dt);

    double startTime = GetTime(); // FOR DEBUG
    PhysicsSystem_Update(reg,bus,dt);
    double endTime = GetTime(); // FOR DEBUG
    physicsTime = (endTime - startTime) * 1000.0; // FOR DEBUG
    
    AnimationSystem_Update(reg,reg->max_used_bound, dt);
    ControlSystem_UpdateCamera(reg);
    ControlSystem_ChangeZoom();

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
    if (CheckSnakeCollision(GRID_WIDTH,GRID_HEIGHT))    SceneManager_ChangeScene(GAME_STATE_GAMEOVER);
}
void Game_Draw(EntityRegistry* reg, CommandBus* bus) {
    //DrawSnake();
    //DrawFood();
    ClearBackground(DARKGREEN);
    cre_RendererCore_BeginWorldMode(creCamera_GetInternal(Viewport_Get()));
    cre_RenderSystem_DrawEntities(reg, creCamera_GetCullBounds(Viewport_Get()));
    DebugSystem_RenderWorldSpace(reg); // World-space debug overlays (inside camera)
    cre_RendererCore_EndWorldMode(); // After this is UI etc.

    // Screen-space debug HUD (outside camera)
    DebugSystem_RenderScreenSpace(reg); // DEBUG
    DebugSystem_RenderMouseHover(reg); // DEBUG

    // For ScoreHUD
    char scoreText[20];
    sprintf(scoreText,"Score: %d", finalScore);
    ViewportSize v = Viewport_Get();
    float uiPadding =  v.width * 0.02f;
    DrawText(scoreText, uiPadding, 20, 30, RAYWHITE);
    
    DrawFPS(10, 10); // FOR DEBUG
    uint32_t activeCount = DebugSystem_GetActiveCount(reg); // FOR DEBUG
    DrawText(TextFormat("Physics time: %.04f ms | Entities: %d",physicsTime, activeCount),20,20,20,RED); // FOR DEBUG
}
void Game_Shutdown(EntityRegistry* reg, CommandBus* bus) {
    // Cleanup if needed
}
static void ResetGameplay(EntityRegistry* reg, CommandBus* bus) {
    Log(LOG_LVL_DEBUG, "ResetGameplay: Starting...");
    EntityManager_Reset(reg);
    Log(LOG_LVL_DEBUG, "ResetGameplay: EntityManager_Reset done");
    InitSnake(GRID_WIDTH/2,GRID_HEIGHT/2);
    Log(LOG_LVL_DEBUG, "ResetGameplay: InitSnake done");
    InitFood();
    Log(LOG_LVL_DEBUG, "ResetGameplay: InitFood done");
    finalScore = 0;
    // this is for test, change location later on
    ControlSystem_SpawnPlayer(reg,bus);
    Log(LOG_LVL_DEBUG, "ResetGameplay: SpawnPlayer done");
}


