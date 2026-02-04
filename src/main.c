#include "engine/core/engine.h"
#define _ISOC11_SOURCE // for aligned_alloc
#include <stdlib.h>
#include "engine/core/config.h"
#include "game_config.h"
#include "engine/core/scene_manager.h"
#include "game/game_scenes.h"
#include "game/game.h"
#include "engine/core/command_bus.h"
#include "engine/core/entity_registry.h"

int main(void) {
    // Using alloc because EntityRegistry is big.
    size_t allocSize = (sizeof(EntityRegistry) + 63) & ~63;
    EntityRegistry* reg = aligned_alloc(64,allocSize);
    if (!reg) return -1;

    CommandBus bus;
    float dt = 0.0166f;

    Engine_Init(reg, &bus, GAME_TITLE, dirCONFIG);
    SceneManager_Init(Game_GetScene);
    SceneManager_ChangeScene(GAME_STATE_PLAYING);
    Engine_Run(reg, &bus, dt);
    Engine_Shutdown(reg, &bus);

    free(reg);
    return 0;
}
