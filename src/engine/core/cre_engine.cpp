#include "cre_engine.h"
#include "cre_commandBus.h"
#include "cre_config.h"
#include "cre_logger.h"
#include "engine/core/cre_enginePackets.h"
#include "engine/core/cre_systemPackets.h"
#include "engine/core/cre_types.h"
#include "engine/ecs/cre_entityManager.h"
#include "engine/ecs/cre_entitySystem.h"
#include "engine/loaders/cre_assetManager.h"
#include "engine/memory/cre_arena.h"
#include "engine/platform/cre_input.h"
#include "engine/platform/cre_sys.h"
#include "engine/platform/cre_time.h"
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

static void EnginePhase0_PlatformSync(p0Packet *packet);
static void EnginePhase1_InputAndLogic(p1Packet *packet);
static void EnginePhase2_Simulation(p2Packet *packet);
static void EnginePhase3_RenderState(p3Packet *packet);
static void EnginePhase4_Cleanup(p4Packet *packet);

void Engine_Init(EngineContext &ctx, const char *title,
                 const char *configFileName) {
  // SubArena Allocations
  ctx.entityArena = arena_Split(&ctx.masterArena, 8 * 1024 * 1024, 64);  // 8MB
  ctx.physicsArena = arena_Split(&ctx.masterArena, 4 * 1024 * 1024, 64); // 4MB
  ctx.busArena = arena_Split(&ctx.masterArena, 8 * 1024 * 1024, 64);     // 8MB
  ctx.audioArena = arena_Split(&ctx.masterArena, 16 * 1024 * 1024, 64);  // 16MB
  ctx.frameArena = arena_Split(&ctx.masterArena, 16 * 1024 * 1024, 64);  // 16MB
  ctx.reg = arena_Push<EntityRegistry>(&ctx.entityArena);
  ctx.bus = arena_Push<CommandBus>(&ctx.busArena);

  timeSystem_Init(&ctx.time);
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
      TextFormat("%s%s", Platform_GetAppDir(), configFileName);
  Input_Init(configPath);
  if (!IsWindowReady()) {
    Log(LogLevel::Error, "[ENGINE] CRITICAL: Raylib failed to create window. ");
    Logger_Shutdown();
    exit(1);
  }

  CommandBus_Init(*ctx.bus);
  EntityManager_Init(*ctx.reg);
  Asset_Init();

  rendererCore_Init(v.width, v.height);
  PhysicsSystem_Init();
  cameraSystem_Init(*ctx.reg);
  audioSystem_Init();
  Log(LogLevel::Info, "[ENGINE] Windows created successfully.");
}
void Engine_Run(EngineContext &ctx) {
  // Engine Phase Packets
  p0Packet pkt0 = {.time = &ctx.time};
  p1Packet pkt1 = {.reg = ctx.reg, .bus = ctx.bus, .time = &ctx.time};
  p2Packet pkt2 = {.reg = ctx.reg, .bus = ctx.bus, .time = &ctx.time};
  p3Packet pkt3 = {.reg = ctx.reg, .bus = ctx.bus, .time = &ctx.time};
  p4Packet pkt4 = {.bus = ctx.bus};

  while (!WindowShouldClose()) {
    PROFILE_START(PROF_TOTAL_ACTIVE);
    EnginePhase0_PlatformSync(&pkt0);
    EnginePhase1_InputAndLogic(&pkt1);
    EnginePhase2_Simulation(&pkt2);
    EnginePhase3_RenderState(&pkt3);
    EnginePhase4_Cleanup(&pkt4);
    arena_Clear(&ctx.frameArena);
    PROFILE_END(PROF_TOTAL_ACTIVE);
    Profiler_UpdateAndPrint(ctx.time.realDt);
  }
}
void Engine_Shutdown(EngineContext &ctx) {
  Log(LogLevel::Info, "Engine Shutting down...");
  SceneManager_Shutdown(*ctx.reg, *ctx.bus);
  EntityManager_Shutdown(*ctx.reg);
  audioSystem_Shutdown();

  CloseWindow();

  // Add failsafes later on.
  Log(LogLevel::Info, "Engine Shutdown Complete.");
  Logger_Shutdown();
}

// ENGINE PHASES
static void EnginePhase0_PlatformSync(p0Packet *packet) {
  timeSystem_Update(packet->time);

  Viewport_Update();
  if (Viewport_wasResized()) {
    ViewportSize vp = Viewport_Get();

    rendererCore_RecreateCanvas(vp.width, vp.height);
    Log(LogLevel::Info, "[ENGINE] Resolution updated to {}x{}", vp.width,
        vp.height);
  }
}

static void EnginePhase1_InputAndLogic(p1Packet *packet) {
  packet->bus->consumed_end = packet->bus->tail;
#ifndef NDEBUG
  packet->bus->current_phase = BUS_PHASE_OPEN;
#endif
  // -----------------------------------------------------------------------------
  scenePacket scenePkt =
      CreateScenePacket(packet->reg, packet->bus, packet->time->gameDt);
  Input_Poll(); // Empty right now.
  // SceneManager handles input right now due to raylib.
  PROFILE_START(PROF_SCENE);
  SceneManager_Update(&scenePkt);
  PROFILE_END(PROF_SCENE);
}

static void EnginePhase2_Simulation(p2Packet *packet) {
  // AI and Particle systems are not implemented right now.
#ifndef NDEBUG
  packet->bus->current_phase = BUS_PHASE_SIMULATION;
#endif

  entityPacket entityPkt = CreateEntityPacket(packet->reg, packet->bus);
  physicsPacket physicsPkt =
      CreatePhysicsPacket(packet->reg, packet->bus, packet->time->fixedDt);
  animPacket animPkt =
      CreateAnimPacket(packet->reg, packet->bus, packet->time->gameDt);

  PROFILE_START(PROF_ECS_SYS);
  EntitySystem_Update(&entityPkt);
  PROFILE_END(PROF_ECS_SYS);

  PROFILE_START(PROF_PHYSICS);
  while (timeSystem_ConsumeFixedStep(packet->time)) {
    PhysicsSystem_Update(&physicsPkt);
  }
  PROFILE_END(PROF_PHYSICS);

  PROFILE_START(PROF_ANIMATION);
  AnimationSystem_Update(&animPkt);
  PROFILE_END(PROF_ANIMATION);

  audioSystem_Update(*packet->bus);
}

static void EnginePhase3_RenderState(p3Packet *packet) {
#ifndef NDEBUG
  packet->bus->current_phase = BUS_PHASE_RENDER;
#endif

  PROFILE_START(PROF_CAMERA);
  cameraPacket camPkt =
      CreateCameraPacket(packet->reg, packet->bus, packet->time->gameDt);
  scenePacket scenePkt = CreateScenePacket(packet->reg, packet->bus, packet->time->gameDt);
  cameraSystem_Update(&camPkt);
  PROFILE_END(PROF_CAMERA);

  PROFILE_START(PROF_RENDER);
  rendererCore_BeginFrame();
  SceneManager_Draw(&scenePkt);
  PROFILE_END(PROF_RENDER);
  rendererCore_EndFrame();
}

static void EnginePhase4_Cleanup(p4Packet *packet) {
  PROFILE_START(PROF_CLEANUP);
#ifndef NDEBUG
  packet->bus->current_phase = BUS_PHASE_OPEN;
#endif

  uint32_t flush_end = packet->bus->consumed_end;

#ifndef NDEBUG
  assert((flush_end - packet->bus->tail) <=
             (packet->bus->head - packet->bus->tail) &&
         "consumed_end outside [tail..head] range");
#endif

  if ((flush_end - packet->bus->tail) >
      (packet->bus->head - packet->bus->tail)) {
    flush_end = packet->bus->tail;
  }

  CommandIterator iter = {.current = packet->bus->tail, .end = flush_end};
  CommandBus_Flush(*packet->bus, &iter);
  PROFILE_END(PROF_CLEANUP);
}
