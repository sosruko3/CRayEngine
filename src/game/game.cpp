#include "game.h"
#include "controlSystem.h"
#include "engine/ecs/cre_entityManager.h"
#include "engine/ecs/cre_entityRegistry.h"
#include "engine/ecs/cre_entitySystem.h"
#include "engine/platform/cre_input.h"
#include "engine/platform/cre_viewport.h"
#include "engine/scene/cre_sceneManager.h"
#include "engine/systems/camera/cre_cameraAPI.h"
#include "engine/systems/camera/cre_cameraSystem.h"
#include "engine/systems/debug/cre_debugSystem.h"
#include "engine/systems/physics/cre_physicsSystem.h"
#include "engine/systems/render/cre_renderAPI.h"
#include "engine/systems/render/cre_renderSystem.h"
#include "engine/systems/render/cre_rendererCore.h"
#include "game_prototypes.h"
#include "game_scenes.h"
#include <assert.h>

// Helper function
static void ResetGameplay(EntityRegistry &reg, CommandBus &bus);

void Game_Init(EntityRegistry &reg, CommandBus &bus) {
  renderAPI_SetDepthPreset(bus, DEPTH_PRESET_FLAT);
  EntitySystem_ClearAllHooks(reg);
  EntityManager_Reset(reg);

  Prototypes_Init(reg);
  ResetGameplay(reg, bus);
}
void Game_Update(EntityRegistry &reg, CommandBus &bus, float dt) {
  ViewportSize vp = Viewport_Get();
  const CameraComponent *activeCam = cameraSystem_GetActiveComponent(reg);
  creRectangle cullBounds =
      cameraSystem_GetActiveCullBounds(reg, activeCam, vp);

  ControlSystem_UpdateSleepState(reg, activeCam);
  DebugSystem_HandleInput(reg);
  ControlSystem_HandleDebugSpawning(reg, bus);
  ControlSystem_UpdateLogic(reg, dt, cullBounds);
  ControlSystem_ChangeZoom(reg, bus, dt);

  if (Input_IsPressed(ACTION_CONFIRM))
    SceneManager_ChangeScene(GAME_STATE_GAMEOVER);
}
void Game_Draw(EntityRegistry &reg, CommandBus &bus) {
  ClearBackground(DARKGREEN); // Use wrapper for this.

  ViewportSize vp = Viewport_Get();
  const CameraComponent *activeCam = cameraSystem_GetActiveComponent(reg);
  creRectangle cullBounds =
      cameraSystem_GetActiveCullBounds(reg, activeCam, vp);

  rendererCore_BeginWorldMode(activeCam, vp);
  // WORLD RENDERING:

  renderSystem_Draw(reg, bus, cullBounds);
  DebugSystem_RenderWorldSpace(reg);
  // World-space debug overlays (inside camera)

  rendererCore_EndWorldMode();
  rendererCore_EndWorldRender();
  // UI RENDERING:

  // Screen-space debug HUD (outside camera)
  DebugSystem_RenderScreenSpace(reg);
}
void Game_Shutdown(EntityRegistry &reg, CommandBus &bus) {
  (void)reg;
  (void)bus;
  // Cleanup if needed
}
static void ResetGameplay(EntityRegistry &reg, CommandBus &bus) {
  Entity mainCam = ControlSystem_SpawnCamera(reg);
  Entity player = ControlSystem_SpawnPlayer(reg, bus);
  ControlSystem_SetCameraTarget(reg, bus, player, mainCam);
}
