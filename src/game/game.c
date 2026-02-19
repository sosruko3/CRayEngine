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
#include "engine/systems/debug/cre_debugSystem.h"
#include "engine/systems/render/cre_renderSystem.h"
#include "engine/systems/physics/cre_physicsSystem.h"
#include "engine/loaders/cre_assetManager.h"
#include "engine/systems/animation/cre_animationSystem.h"
#include "engine/systems/render/cre_rendererCore.h"
#include "engine/platform/cre_viewport.h"
#include "engine/systems/camera/cre_cameraSystem.h"
#include "engine/platform/cre_input.h"
#include <assert.h>

// Global variables
static EntityRegistry* s_gameReg = NULL; // Store registry pointer for ResetGameplay

// Helper function
 static void ResetGameplay(EntityRegistry* reg, CommandBus* bus);

void Game_Init(EntityRegistry* reg, CommandBus* bus) {
    assert (reg || bus);
    s_gameReg = reg; // Check this part.
    ResetGameplay(reg, bus);
}
void Game_Update(EntityRegistry* reg, CommandBus* bus,float dt) {
    ControlSystem_UpdateSleepState(reg);
    DebugSystem_HandleInput(reg);
    ControlSystem_HandleDebugSpawning(reg, bus);
    ControlSystem_UpdateLogic(reg, dt);
    ControlSystem_ChangeZoom(dt);

    if (Input_IsPressed(ACTION_CONFIRM)) SceneManager_ChangeScene(GAME_STATE_GAMEOVER);
}
void Game_Draw(EntityRegistry* reg, CommandBus* bus) {
    ClearBackground(DARKGREEN); // Use wrapper for this.

    rendererCore_BeginWorldMode(cameraSystem_GetInternal());
    // WORLD RENDERING:

    renderSystem_Draw(reg,cameraSystem_GetCullBounds());
    DebugSystem_RenderWorldSpace(reg); // World-space debug overlays (inside camera)
    
    rendererCore_EndWorldMode(); 
    rendererCore_EndWorldRender();
    // UI RENDERING:
    
    // Screen-space debug HUD (outside camera)
    DebugSystem_RenderScreenSpace(reg);
    DebugSystem_RenderMouseHover(reg);

    ViewportSize v = Viewport_Get();
    DrawFPS(v.width*0.02f,v.height*0.02f); // FOR DEBUG
    uint32_t activeCount = DebugSystem_GetActiveCount(reg); // FOR DEBUG
    DrawText(TextFormat("Physics time: %.04f ms | Entities: %d",activeCount, activeCount),20,20,20,RED); // FOR DEBUG
}
void Game_Shutdown(EntityRegistry* reg, CommandBus* bus) {
    // Cleanup if needed
}
static void ResetGameplay(EntityRegistry* reg, CommandBus* bus) {
    EntityManager_Reset(reg); // Create an command for this, do not use this directly.
    
    ControlSystem_SpawnPlayer(reg,bus);
}


