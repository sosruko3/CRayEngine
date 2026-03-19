#include "engine/core/cre_engine.h"
#define _ISOC11_SOURCE // for aligned_alloc
#include "engine/core/cre_commandBus.h"
#include "engine/ecs/cre_entityRegistry.h"
#include "engine/scene/cre_sceneManager.h"
#include "game/game.h"
#include "game/game_scenes.h"
#include "game_config.h"
#include <stdlib.h>

int main(void) {
  size_t regAllocSize =
      (sizeof(EntityRegistry) + 63) & ~(static_cast<size_t>(63));
  EntityRegistry *reg =
      static_cast<EntityRegistry *>(aligned_alloc(64, regAllocSize));
  if (!reg)
    return -1;

  size_t busAllocSize = (sizeof(CommandBus) + 63) & ~(static_cast<size_t>(63));
  CommandBus *bus = static_cast<CommandBus *>(aligned_alloc(64, busAllocSize));
  if (!bus)
    return -1;

  float dt = 0.0166f;

  Engine_Init(reg, bus, GAME_TITLE, dirCONFIG);
  SceneManager_Init(Game_GetScene);
  SceneManager_ChangeScene(GAME_STATE_PLAYING);
  Engine_Run(reg, bus, dt);
  Engine_Shutdown(reg, bus);

  free(bus);
  free(reg);
  return 0;
}
