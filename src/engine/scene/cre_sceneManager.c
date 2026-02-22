#include "cre_sceneManager.h"
#include "engine/core/cre_logger.h"
#include "engine/ecs/cre_entitySystem.h"
#include "stddef.h" // for NULL
#include "stdbool.h"

// Internal state
typedef struct {
    Scene        currentScene;
    int          activeState;
    int          nextState;
    bool         isSwitchPending;
    SceneFactory factory;
} SceneManagerContext;

static SceneManagerContext ctx = {
    .activeState     = -1,
    .isSwitchPending = false,
    .factory = NULL
};
// Public API
void SceneManager_Init(SceneFactory factory) {
    ctx.factory = factory;
    ctx.activeState = -1;
    ctx.isSwitchPending = false;
    Log(LOG_LVL_INFO,"Scene Manager Initialized.");
}

void SceneManager_Update(EntityRegistry* reg, CommandBus* bus, float dt) {
    if (ctx.isSwitchPending) {
        if (ctx.currentScene.Unload)  ctx.currentScene.Unload(reg, bus);
        EntitySystem_ClearCloneHooks(reg);
        
        if (ctx.factory) {
            ctx.currentScene = ctx.factory(ctx.nextState);
        }
        ctx.activeState = ctx.nextState;
        if (ctx.currentScene.Init) ctx.currentScene.Init(reg, bus);
        ctx.isSwitchPending = false;
    }
    if (ctx.currentScene.Update) {
        ctx.currentScene.Update(reg, bus, dt);
    }
}
void SceneManager_Draw(EntityRegistry* reg, CommandBus* bus) {
    if (ctx.currentScene.Draw) {
        ctx.currentScene.Draw(reg, bus);
    }
}

void SceneManager_Shutdown(EntityRegistry* reg, CommandBus* bus) {
    if (ctx.currentScene.Unload) {
        ctx.currentScene.Unload(reg, bus);
    }
    EntitySystem_ClearCloneHooks(reg);
}

void SceneManager_ChangeScene(int nextState) {
    ctx.nextState = nextState;
    ctx.isSwitchPending = true;
    Log(LOG_LVL_INFO, "Scene changing queued for next frame...");
}

int SceneManager_GetActiveState(void) {
    return ctx.activeState;
}