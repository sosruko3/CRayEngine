#include "cre_engine.h"
#include "raylib.h"
#include "cre_config.h"
#include "cre_logger.h"
#include "engine/platform/cre_input.h"
#include "engine/scene/cre_sceneManager.h"
#include "engine/ecs/cre_entityManager.h"
#include "engine/systems/physics/cre_physicsSystem.h"
#include "engine/systems/render/cre_RendererCore.h"
#include "engine/loaders/cre_assetManager.h"
#include "engine/platform/cre_viewport.h"
#include "engine/systems/camera/cre_cameraSystem.h"
#include "cre_commandBus.h"
#include "engine/core/cre_enginePhases.h"
#include <stdio.h>
#include <stdlib.h>

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
    RendererCore_Init((int)v.width,(int)v.height);

    CommandBus_Init(bus); // Make sure this function is getting called.
    EntityManager_Init(reg);
    Asset_Init();
    PhysicsSystem_Init();

    cameraSystem_Init(Viewport_Get());
    Log(LOG_LVL_INFO,"Windows created successfully.");
}
void Engine_Run(EntityRegistry* reg, CommandBus* bus,float dt) {
    while (!WindowShouldClose()) {
        EnginePhase0_PlatformSync();
        EnginePhase1_InputAndLogic(reg,bus,dt);
        EnginePhase2_Simulation(reg,bus,dt);
        EnginePhase3_RenderState(reg,bus,dt);
        CommandIterator iter = CommandBus_GetIterator(bus);
        CommandBus_Flush(bus,&iter);
    }
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
