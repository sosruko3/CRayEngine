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
#include "asset_manager.h"
#include "viewport.h"

void Engine_Init(int width, int height, const char* title, const char* configFileName) {
    Logger_Init();
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);  // FOR DEBUG
    InitWindow(width,height, title);
    SetTargetFPS(TARGET_FRAMERATE);
    Viewport_Init(GetScreenWidth(),GetScreenHeight(),GAME_VIRTUAL_HEIGHT);
    ViewportSize v = Viewport_Get();
    Log(LOG_LVL_INFO,"Engine Initializing...");
    Log(LOG_LVL_DEBUG,"Target Resolution: %dx%d",width,height);

    const char* configPath = TextFormat("%s%s", GetApplicationDirectory(), configFileName);
    Input_Init(configPath);
    if (!IsWindowReady()) {
        Log(LOG_LVL_ERROR,"CRITICAL: Raylib failed to create window. ");
        Logger_Shutdown();
        exit(1);
    }
    creRenderer_Init((int)v.width,(int)v.height);
    EntityManager_Init();
    Asset_Init();
    PhysicsSystem_Init();
    Log(LOG_LVL_INFO,"Windows created successfully.");
}
void Engine_Run() {
    Log(LOG_LVL_INFO,"Entering main loop");
    while (!WindowShouldClose()) {
        
        if (Viewport_ShouldResize()) {
            ViewportSize v = Viewport_Get();
            creRenderer_Init((int)v.width,(int)v.height);
            Log(LOG_LVL_INFO,"ENGINE: Resolution updated to %0.fx%0.f",v.width,v.height);
        }
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
