#include "engine/core/cre_commandBus.h"
#include "engine/core/cre_engine.h"
#include "engine/core/cre_types.h"
#include "engine/ecs/cre_entityRegistry.h"
#include "engine/scene/cre_sceneManager.h"
#include "engine/memory/cre_arena.h"
#include "game/game.h"
#include "game/game_scenes.h"
#include "game_config.h"

int main() {
  Arena masterArena = arena_AllocateMemory(256 * 1024 * 1024); // 256MB
  EngineContext ctx = {};
  ctx.masterArena = masterArena;
  if (!ctx.masterArena.base_ptr) {
    arena_FreeMemory(&ctx.masterArena);
    return -1;
  }
  float dt = 0.0166f; // temp value
  Engine_Init(ctx, GAME_TITLE, dirCONFIG);
  SceneManager_Init(Game_GetScene);
  SceneManager_ChangeScene(GAME_STATE_PLAYING);
  Engine_Run(ctx, dt);
  Engine_Shutdown(ctx);

  arena_FreeMemory(&ctx.masterArena);
  return 0;
}
