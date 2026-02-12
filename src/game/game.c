#include "game.h"
#include "engine/core/cre_config.h"
#include "engine/core/cre_uiHelper.h"
#include "raylib.h"
#include "../game_config.h"
#include "engine/scene/cre_sceneManager.h"
#include "game_scenes.h"
#include "engine/ecs/cre_entityManager.h"
#include "engine/ecs/cre_entityRegistry.h"
#include "engine/core/cre_logger.h"
#include "entity_types.h"
#include "controlSystem.h"
#include "debugSystem.h"
#include "engine/systems/render/cre_RenderSystem.h"
#include "engine/systems/physics/cre_physicsSystem.h"
#include "engine/loaders/cre_assetManager.h"
#include "engine/systems/animation/cre_animationSystem.h"
#include "engine/systems/render/cre_RendererCore.h"
#include "engine/platform/cre_viewport.h"
#include "engine/systems/camera/cre_camera.h"
#include "engine/platform/cre_input.h"
#include <stdio.h> // For sprintf
#include <assert.h>

// Global variables
static double physicsTime = 0.0; // FOR DEBUG
static EntityRegistry* s_gameReg = NULL; // Store registry pointer for ResetGameplay

// Helper function
 static void ResetGameplay(EntityRegistry* reg, CommandBus* bus);

void Game_Init(EntityRegistry* reg, CommandBus* bus) {
    assert (reg || bus);
    s_gameReg = reg;
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
    
    AnimationSystem_Update(reg, dt);
    ControlSystem_UpdateCamera(reg);
    ControlSystem_ChangeZoom();

    if (Input_IsPressed(ACTION_CONFIRM))    SceneManager_ChangeScene(GAME_STATE_GAMEOVER);
}
void Game_Draw(EntityRegistry* reg, CommandBus* bus) {

    ClearBackground(DARKGREEN);
    cre_RendererCore_BeginWorldMode(creCamera_GetInternal(Viewport_Get()));
    cre_RenderSystem_DrawEntities(reg, creCamera_GetCullBounds(Viewport_Get()));
    DebugSystem_RenderWorldSpace(reg); // World-space debug overlays (inside camera)
    cre_RendererCore_EndWorldMode(); // After this is UI etc.

    // Screen-space debug HUD (outside camera)
    DebugSystem_RenderScreenSpace(reg); // DEBUG
    DebugSystem_RenderMouseHover(reg); // DEBUG

    ViewportSize v = Viewport_Get();
    DrawFPS(v.width*0.02f,v.height*0.02f); // FOR DEBUG
    uint32_t activeCount = DebugSystem_GetActiveCount(reg); // FOR DEBUG
    DrawText(TextFormat("Physics time: %.04f ms | Entities: %d",physicsTime, activeCount),20,20,20,RED); // FOR DEBUG
}
void Game_Shutdown(EntityRegistry* reg, CommandBus* bus) {
    // Cleanup if needed
}
static void ResetGameplay(EntityRegistry* reg, CommandBus* bus) {
    Log(LOG_LVL_DEBUG, "ResetGameplay: Starting...");
    EntityManager_Reset(reg);
    
    ControlSystem_SpawnPlayer(reg,bus);
    Log(LOG_LVL_DEBUG, "ResetGameplay: SpawnPlayer done");
}


