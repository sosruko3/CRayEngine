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
#include "cre_RendererCore.h"
#include "asset_manager.h"
#include "viewport.h"
#include "cre_camera.h"
#include "command_bus.h"

void Engine_Init(EntityRegistry* reg, CommandBus* bus,const char* title, const char* configFileName) {
    Logger_Init();

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);  // FOR DEBUG
    Viewport_Init(SCREEN_WIDTH,SCREEN_HEIGHT);
    ViewportSize v = Viewport_Get();
    InitWindow(v.width,v.height,title);
    SetTargetFPS(TARGET_FRAMERATE);
    Log(LOG_LVL_INFO,"Engine Initializing...");
    Log(LOG_LVL_DEBUG,"Target Resolution: %.0fx%.0f",v.width,v.height);

    const char* configPath = TextFormat("%s%s", GetApplicationDirectory(), configFileName);
    Input_Init(configPath);
    if (!IsWindowReady()) {
        Log(LOG_LVL_ERROR,"CRITICAL: Raylib failed to create window. ");
        Logger_Shutdown();
        exit(1);
    }
    cre_RendererCore_Init((int)v.width,(int)v.height);

    CommandBus_Init(bus); // Make sure this function is getting called.
    EntityManager_Init(reg);
    Asset_Init();
    PhysicsSystem_Init();

    creCamera_Init(Viewport_Get());
    Log(LOG_LVL_INFO,"Windows created successfully.");
}
void Engine_Run(EntityRegistry* reg, CommandBus* bus,float dt) {
    Log(LOG_LVL_INFO,"Entering main loop");
    
    while (!WindowShouldClose()) {
        Viewport_Update();
        if (Viewport_wasResized()) {
            ViewportSize vp = Viewport_Get();
            creCamera_UpdateViewportCache(vp);
            cre_RendererCore_RecreateCanvas((int)vp.width,(int)vp.height);
            Log(LOG_LVL_INFO,"ENGINE: Resolution updated to %0.fx%0.f",vp.width,vp.height);
        }
        SceneManager_Update(reg,bus,dt);

        cre_RendererCore_BeginFrame();
        SceneManager_Draw(reg,bus);
        cre_RendererCore_EndFrame();
    }
    Log(LOG_LVL_INFO,"Main loop exited. (Window closed)");
}
void Engine_Shutdown(EntityRegistry* reg, CommandBus* bus) {
    Log(LOG_LVL_INFO,"Shutting down Raylib...");
    SceneManager_Shutdown(reg,bus);
    EntityManager_Shutdown(reg);
    CloseWindow();

    // Add failsafes later on.
    Log(LOG_LVL_INFO,"Engine Shutdown Complete.");
    Logger_Shutdown();
}
