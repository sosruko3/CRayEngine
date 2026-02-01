#include "engine/core/engine.h"
#include "engine/core/config.h"
#include "game_config.h"
#include "engine/core/scene_manager.h"
#include "game/game_scenes.h"
#include "game/game.h"
#include "engine/core/command_bus.h"
#include "engine/core/entity_registry.h"

int main(void) {
    EntityRegistry reg;
    CommandBus bus;
    float dt = 0.0166f;
    Engine_Init(&reg, &bus, GAME_TITLE, dirCONFIG);
    SceneManager_Init(Game_GetScene);
    SceneManager_ChangeScene(GAME_STATE_PLAYING);
    Engine_Run(&reg, &bus, dt);
    Engine_Shutdown(&reg, &bus);
    return 0;
}
