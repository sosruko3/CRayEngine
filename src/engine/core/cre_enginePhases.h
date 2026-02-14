#ifndef CRE_ENGINEPHASES_H
#define CRE_ENGINEPHASES_H

#include "engine/scene/cre_sceneManager.h"
#include "engine/platform/cre_input.h"
#include "engine/systems/animation/cre_animationSystem.h"
#include "engine/systems/physics/cre_physicsSystem.h"
#include "engine/systems/camera/cre_camera.h"
#include "engine/systems/camera/cre_cameraSystem.h"
#include "engine/systems/render/cre_RenderSystem.h"
#include "engine/systems/render/cre_RendererCore.h"
#include "engine/ecs/cre_entitySystem.h"
#include "engine/platform/cre_viewport.h"

void EnginePhase0_PlatformSync(void) {
    Viewport_Update();
    if (Viewport_wasResized()) {
        ViewportSize vp = Viewport_Get();

        creCamera_UpdateViewportCache(vp);
        cre_RendererCore_RecreateCanvas((int)vp.width,(int)vp.height);
        Log(LOG_LVL_INFO,"ENGINE: Resolution updated to %0.fx%0.f",vp.width,vp.height);
    }
}

void EnginePhase1_InputAndLogic(EntityRegistry* reg,CommandBus* bus,float dt) {
    Input_Poll(); // Empty right now.

    // SceneManager handles input right now due to raylib.
    SceneManager_Update(reg,bus,dt); 
}

void EnginePhase2_Simulation(EntityRegistry* reg,CommandBus* bus,float dt) {
    // AI and Particle systems are not implemented right now.
    EntitySystem_Update(reg,bus);

    PhysicsSystem_Update(reg,bus,dt);

    AnimationSystem_Update(reg,dt);
}

#endif