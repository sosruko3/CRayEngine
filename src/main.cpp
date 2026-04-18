#include "engine/core/cre_engine.h"
#include "engine/core/cre_types.h"
#include "engine/memory/cre_arena.h"
#include "engine/scene/cre_sceneManager.h"
#include "game/game_scenes.h"
#include "game_config.h"

int main() {

  EngineContext ctx = {};
  ctx.masterArena = arena_AllocateMemory(256 * 1024 * 1024); // 256MB
  if (!ctx.masterArena.base_ptr) {
    arena_FreeMemory(&ctx.masterArena);
    return -1;
  }
  Engine_Init(ctx, GAME_TITLE, dirCONFIG);
  SceneManager_Init(Game_GetScene);
  SceneManager_ChangeScene(GAME_STATE_PLAYING);
  Engine_Run(ctx);
  Engine_Shutdown(ctx);

  arena_FreeMemory(&ctx.masterArena);
  return 0;
}
