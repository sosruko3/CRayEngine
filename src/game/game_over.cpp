#include "game_over.h"
#include "engine/core/cre_config.h"
#include "engine/ecs/cre_entityRegistry.h"
#include "engine/platform/cre_input.h"
#include "engine/platform/cre_viewport.h"
#include "engine/scene/cre_sceneManager.h"
#include "engine/systems/camera/cre_cameraSystem.h"
#include "engine/systems/render/cre_renderSystem.h"
#include "engine/systems/render/cre_rendererCore.h"
#include "game_config.h"
#include "game_scenes.h"

void GameOver_Init(EntityRegistry &reg, CommandBus &bus) {
  (void)reg;
  (void)bus;
  // Optional for game over sound effect or anything
}

void GameOver_Update(EntityRegistry &reg, CommandBus &bus, float dt) {
  (void)reg;
  (void)bus;
  (void)dt;
  if (Input_IsPressed(ACTION_CONFIRM)) {
    SceneManager_ChangeScene(GAME_STATE_MENU);
  }
}

void GameOver_Draw(EntityRegistry &reg, CommandBus &bus) {

  ClearBackground(BLACK);
  ViewportSize vp = Viewport_Get();
  const CameraComponent *activeCam = cameraSystem_GetActiveComponent(reg);
  creRectangle cullBounds =
      cameraSystem_GetActiveCullBounds(reg, activeCam, vp);

  rendererCore_BeginWorldMode(activeCam, vp);

  renderSystem_Draw(reg, bus, cullBounds);

  rendererCore_EndWorldMode();
  rendererCore_EndWorldRender();

  int titleWidth = MeasureText(GAMEOVER_TITLE_TEXT, FONT_SIZE_TITLE);
  DrawText(GAMEOVER_TITLE_TEXT,
           (static_cast<int>(SCREEN_WIDTH) - titleWidth) / 2,
           static_cast<int>(SCREEN_HEIGHT) / 2 - 50, FONT_SIZE_TITLE, RED);

  int restartWidth = MeasureText(GAMEOVER_RESTART_TEXT, FONT_SIZE_SUBTITLE);
  DrawText(GAMEOVER_RESTART_TEXT,
           (static_cast<int>(SCREEN_WIDTH) - restartWidth) / 2,
           static_cast<int>(SCREEN_HEIGHT) / 2 + 50, FONT_SIZE_SUBTITLE,
           RAYWHITE);
}

void GameOver_Unload(EntityRegistry &reg, CommandBus &bus) {
  (void)reg;
  (void)bus;
  // Nothing to clean up yet
}
