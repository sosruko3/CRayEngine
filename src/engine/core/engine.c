#include "engine.h"
#include "raylib.h"
#include <stdio.h>
#include "config.h"
#include "logger.h"
#include <stdlib.h>
#include "input.h"
#include "scene_manager.h"
#include "entity_manager.h"
#include "../physics/physics_system.h"
#include "cre_renderer.h"

void Engine_Init(int width, int height, const char* title, const char* configFileName) {
    Logger_Init();
    Log(LOG_LVL_INFO,"Engine Initializing...");
    Log(LOG_LVL_DEBUG,"Target Resolution: %dx%d",width,height);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);  // FOR DEBUG
    InitWindow(SCREEN_WIDTH,SCREEN_HEIGHT, title); //
    SetTargetFPS(TARGET_FRAMERATE);
    
    const char* configPath = TextFormat("%s%s", GetApplicationDirectory(), configFileName);
    Input_Init(configPath);
    if (!IsWindowReady()) {
        Log(LOG_LVL_ERROR,"CRITICAL: Raylib failed to create window. ");
        Logger_Shutdown();
        exit(1);
    }
    creRenderer_Init(1080);
    EntityManager_Init();
    PhysicsSystem_Init();
    SceneManager_Init();
    Log(LOG_LVL_INFO,"Windows created successfully.");
}
void Engine_Run() {
    Log(LOG_LVL_INFO,"Entering main loop");
    while (!WindowShouldClose()) {
        SceneManager_Update();

        creRenderer_BeginFrame();
        SceneManager_Draw();
        creRenderer_EndFrame();
    }
    Log(LOG_LVL_INFO,"Main loop exited. (Window closed)");
}
void Engine_Shutdown(void) {
    Log(LOG_LVL_INFO,"Shutting down Raylib...");
    SceneManager_Shutdown();
    EntityManager_Shutdown();
    CloseWindow();

    // Add failsafes later on.
    Log(LOG_LVL_INFO,"Engine Shutdown Complete.");
    Logger_Shutdown();
}
