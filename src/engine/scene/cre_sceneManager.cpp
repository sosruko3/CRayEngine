#include "cre_sceneManager.h"
#include "engine/core/cre_logger.h"
#include "engine/core/cre_systemPackets.h"
#include "engine/ecs/cre_entitySystem.h"
#include <stdbool.h>
#include <stdint.h>

// Internal state
typedef struct {
  Scene currentScene;
  int32_t activeState;
  int32_t nextState;
  bool isSwitchPending;
  SceneFactory factory;
} SceneManagerContext;

static SceneManagerContext ctx = {.currentScene = {},
                                  .activeState = -1,
                                  .nextState = -1,
                                  .isSwitchPending = false,
                                  .factory = nullptr};

scenePacket CreateScenePacket(EntityRegistry *reg, CommandBus *bus, float dt) {
  scenePacket pkt = {.reg = reg, .bus = bus, .gameDt = dt};
  return pkt;
}
// Public API
void SceneManager_Init(SceneFactory factory) {
  ctx.factory = factory;
  ctx.activeState = -1;
  ctx.isSwitchPending = false;
  Log(LogLevel::Info, "Scene Manager Initialized.");
}

void SceneManager_Update(scenePacket *packet) {

  if (ctx.isSwitchPending) {
    if (ctx.currentScene.Unload)
      ctx.currentScene.Unload(*packet->reg, *packet->bus);
    EntitySystem_ClearAllHooks(*packet->reg);

    if (ctx.factory) {
      ctx.currentScene = ctx.factory(ctx.nextState);
    }
    ctx.activeState = ctx.nextState;
    if (ctx.currentScene.Init)
      ctx.currentScene.Init(*packet->reg, *packet->bus);
    ctx.isSwitchPending = false;
  }
  if (ctx.currentScene.Update) {
    ctx.currentScene.Update(*packet->reg, *packet->bus, packet->gameDt);
  }
}
void SceneManager_Draw(scenePacket *packet) {
  if (ctx.currentScene.Draw) {
    ctx.currentScene.Draw(*packet->reg, *packet->bus);
  }
}

void SceneManager_Shutdown(EntityRegistry &reg, CommandBus &bus) {
  if (ctx.currentScene.Unload) {
    ctx.currentScene.Unload(reg, bus);
  }
  EntitySystem_ClearAllHooks(reg);
}

void SceneManager_ChangeScene(int32_t nextState) {
  ctx.nextState = nextState;
  ctx.isSwitchPending = true;
  Log(LogLevel::Info, "Scene changing queued for next frame...");
}

int SceneManager_GetActiveState(void) { return ctx.activeState; }
