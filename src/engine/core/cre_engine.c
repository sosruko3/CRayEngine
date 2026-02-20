#include "cre_engine.h"
#include "raylib.h"
#include "cre_config.h"
#include "cre_logger.h"
#include "engine/platform/cre_input.h"
#include "engine/scene/cre_sceneManager.h"
#include "engine/ecs/cre_entityManager.h"
#include "engine/systems/physics/cre_physicsSystem.h"
#include "engine/systems/render/cre_rendererCore.h"
#include "engine/ecs/cre_entitySystem.h"
#include "engine/systems/animation/cre_animationSystem.h"
#include "engine/loaders/cre_assetManager.h"
#include "engine/platform/cre_viewport.h"
#include "engine/systems/camera/cre_cameraSystem.h"
#include "cre_commandBus.h"
#include <stdio.h>
#include <stdlib.h>

// ENGINE PHASES
static void EnginePhase0_PlatformSync(void) {
    Viewport_Update();
    if (Viewport_wasResized()) {
        ViewportSize vp = Viewport_Get();

        cameraSystem_UpdateViewportCache(vp);
        rendererCore_RecreateCanvas((int32_t)vp.width,(int32_t)vp.height);
        Log(LOG_LVL_INFO,"[ENGINE] Resolution updated to %0.fx%0.f",vp.width,vp.height);
    }
}

static void EnginePhase1_InputAndLogic(EntityRegistry* restrict reg,CommandBus* bus,float dt) {
    Input_Poll(); // Empty right now.

    // SceneManager handles input right now due to raylib.
    SceneManager_Update(reg,bus,dt); 
}

static void EnginePhase2_Simulation(EntityRegistry* restrict  reg,CommandBus* bus,float dt) {
    // AI and Particle systems are not implemented right now.
    EntitySystem_Update(reg,bus);
    PhysicsSystem_Update(reg,bus,dt);
    AnimationSystem_Update(reg,dt);
}

static void EnginePhase3_RenderState(EntityRegistry* restrict reg, CommandBus* bus, float dt) {
    cameraSystem_Update(reg,bus,dt);

    rendererCore_BeginFrame();
    SceneManager_Draw(reg,bus);
    rendererCore_EndFrame();
}
static void EnginePhase4_Cleanup(EntityRegistry* restrict reg, CommandBus* bus) {
    CommandIterator iter = CommandBus_GetIterator(bus);
    CommandBus_Flush(bus,&iter);
}

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
    rendererCore_Init((int32_t)v.width,(int32_t)v.height);

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
        EnginePhase4_Cleanup(reg,bus);
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
