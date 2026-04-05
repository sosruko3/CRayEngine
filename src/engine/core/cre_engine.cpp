#include "cre_engine.h"
#include "cre_commandBus.h"
#include "cre_config.h"
#include "cre_logger.h"
#include "engine/ecs/cre_entityManager.h"
#include "engine/ecs/cre_entitySystem.h"
#include "engine/loaders/cre_assetManager.h"
#include "engine/platform/cre_input.h"
#include "engine/platform/cre_viewport.h"
#include "engine/scene/cre_sceneManager.h"
#include "engine/systems/animation/cre_animationSystem.h"
#include "engine/systems/audio/cre_audioSystem.h"
#include "engine/systems/camera/cre_cameraSystem.h"
#include "engine/systems/debug/cre_profilerSystem.h"
#include "engine/systems/physics/cre_physicsSystem.h"
#include "engine/systems/render/cre_rendererCore.h"
#include "raylib.h"
#include <stdlib.h>

// ENGINE PHASES
static void EnginePhase0_PlatformSync(void) {
  Viewport_Update();
  if (Viewport_wasResized()) {
    ViewportSize vp = Viewport_Get();

    rendererCore_RecreateCanvas(vp.width, vp.height);
    Log(LogLevel::Info, "[ENGINE] Resolution updated to {}x{}", vp.width,
        vp.height);
  }
}
static void EnginePhase1_InputAndLogic(EntityRegistry &reg, CommandBus &bus,
                                       float dt) {
  // Note: If you use command bus on phase0 , you need to move these to first
  // part of phase0
  bus.consumed_end = bus.tail;
#ifndef NDEBUG
  bus.current_phase = BUS_PHASE_OPEN;
#endif

  Input_Poll(); // Empty right now.

  // SceneManager handles input right now due to raylib.
  PROFILE_START(PROF_SCENE);
  SceneManager_Update(reg, bus, dt);
  PROFILE_END(PROF_SCENE);
}
static void EnginePhase2_Simulation(EntityRegistry &reg, CommandBus &bus,
                                    float dt) {
  // AI and Particle systems are not implemented right now.
#ifndef NDEBUG
  bus.current_phase = BUS_PHASE_SIMULATION;
#endif

  PROFILE_START(PROF_ECS_SYS);
  EntitySystem_Update(reg, bus);
  PROFILE_END(PROF_ECS_SYS);

  PROFILE_START(PROF_PHYSICS);
  PhysicsSystem_Update(reg, bus, dt);
  PROFILE_END(PROF_PHYSICS);

  PROFILE_START(PROF_ANIMATION);
  AnimationSystem_Update(reg, bus, dt);
  PROFILE_END(PROF_ANIMATION);

  audioSystem_Update(reg, bus);
}
static void EnginePhase3_RenderState(EntityRegistry &reg, CommandBus &bus,
                                     float dt) {
#ifndef NDEBUG
  bus.current_phase = BUS_PHASE_RENDER;
#endif

  PROFILE_START(PROF_CAMERA);
  cameraSystem_Update(reg, bus, dt, Viewport_Get());
  PROFILE_END(PROF_CAMERA);

  PROFILE_START(PROF_RENDER);
  rendererCore_BeginFrame();
  SceneManager_Draw(reg, bus);
  PROFILE_END(PROF_RENDER);
  rendererCore_EndFrame();
}
static void EnginePhase4_Cleanup(EntityRegistry &reg, CommandBus &bus) {
  PROFILE_START(PROF_CLEANUP);
  (void)reg;
#ifndef NDEBUG
  bus.current_phase = BUS_PHASE_OPEN;
#endif

  uint32_t flush_end = bus.consumed_end;

#ifndef NDEBUG
  assert((flush_end - bus.tail) <= (bus.head - bus.tail) &&
         "consumed_end outside [tail..head] range");
#endif

  if ((flush_end - bus.tail) > (bus.head - bus.tail)) {
    flush_end = bus.tail;
  }

  CommandIterator iter = {.current = bus.tail, .end = flush_end};
  CommandBus_Flush(bus, &iter);
  PROFILE_END(PROF_CLEANUP);
}

void Engine_Init(EntityRegistry &reg, CommandBus &bus, const char *title,
                 const char *configFileName) {
  Logger_Init();
  Log(LogLevel::Info, "[ENGINE] Engine is Initializing...");

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  Viewport_Init(SCREEN_WIDTH, SCREEN_HEIGHT);
  ViewportSize v = Viewport_Get();
  InitWindow(static_cast<int>(v.width), static_cast<int>(v.height), title);
  SetTargetFPS(TARGET_FRAMERATE);
  Log(LogLevel::Debug, "[ENGINE] Target Resolution: {}x{}", v.width, v.height);

  // Clean this configPath later on.
  const char *configPath =
      TextFormat("%s%s", GetApplicationDirectory(), configFileName);
  Input_Init(configPath);
  if (!IsWindowReady()) {
    Log(LogLevel::Error, "[ENGINE] CRITICAL: Raylib failed to create window. ");
    Logger_Shutdown();
    exit(1);
  }

  CommandBus_Init(bus); // Make sure this function is getting called.
  EntityManager_Init(reg);
  Asset_Init();

  rendererCore_Init(v.width, v.height);
  PhysicsSystem_Init();
  cameraSystem_Init(reg);
  audioSystem_Init();
  Log(LogLevel::Info, "[ENGINE] Windows created successfully.");
}
void Engine_Run(EntityRegistry &reg, CommandBus &bus, float dt) {
  while (!WindowShouldClose()) {
    PROFILE_START(PROF_TOTAL_ACTIVE);
    EnginePhase0_PlatformSync();
    EnginePhase1_InputAndLogic(reg, bus, dt);
    EnginePhase2_Simulation(reg, bus, dt);
    EnginePhase3_RenderState(reg, bus, dt);
    EnginePhase4_Cleanup(reg, bus);
    PROFILE_END(PROF_TOTAL_ACTIVE);
    Profiler_UpdateAndPrint(dt);
  }
}
void Engine_Shutdown(EntityRegistry &reg, CommandBus &bus) {
  Log(LogLevel::Info, "[ENGINE] Shutting down...");
  SceneManager_Shutdown(reg, bus);
  EntityManager_Shutdown(reg);
  audioSystem_Shutdown();

  CloseWindow();

  // Add failsafes later on.
  Log(LogLevel::Info, "[ENGINE] Engine Shutdown Complete.");
  Logger_Shutdown();
}
