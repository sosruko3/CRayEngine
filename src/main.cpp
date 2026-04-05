#include "engine/core/cre_commandBus.h"
#include "engine/core/cre_engine.h"
#include "engine/ecs/cre_entityRegistry.h"
#include "engine/scene/cre_sceneManager.h"
#include "game/game.h"
#include "game/game_scenes.h"
#include "game_config.h"
#include <stdalign.h>
#include <cstdlib>

int main() {
  EntityRegistry *ptrReg = static_cast<EntityRegistry *>(
      std::aligned_alloc(alignof(EntityRegistry), sizeof(EntityRegistry)));
  CommandBus *ptrBus = static_cast<CommandBus *>(
      std::aligned_alloc(alignof(CommandBus), sizeof(CommandBus)));

  if (!ptrReg || !ptrBus) {
    std::free(ptrReg);
    std::free(ptrBus);
    return -1;
  }
  EntityRegistry &reg = *static_cast<EntityRegistry *>(ptrReg);
  CommandBus &bus = *static_cast<CommandBus *>(ptrBus);

  float dt = 0.0166f;

  Engine_Init(reg, bus, GAME_TITLE, dirCONFIG);
  SceneManager_Init(Game_GetScene);
  SceneManager_ChangeScene(GAME_STATE_PLAYING);
  Engine_Run(reg, bus, dt);
  Engine_Shutdown(reg, bus);

  std::free(ptrBus);
  std::free(ptrReg);
  return 0;
}
