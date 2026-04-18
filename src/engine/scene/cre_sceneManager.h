#ifndef CRE_SCENEMANAGER_H
#define CRE_SCENEMANAGER_H

#include "cre_scenes.h"
#include <stdint.h>

// Forward declarations (cleaner than including headers)
struct EntityRegistry;
struct CommandBus;
struct scenePacket;

typedef Scene (*SceneFactory)(int32_t stateID);

scenePacket CreateScenePacket(EntityRegistry *reg, CommandBus *bus, float dt);
// Start
void SceneManager_Init(SceneFactory factory);

// Main loop calls these
void SceneManager_Update(scenePacket *packet);
void SceneManager_Draw(scenePacket *packet);

// Clean up whatever scene is currently active
void SceneManager_Shutdown(EntityRegistry &reg, CommandBus &bus);

// Switch scenes
void SceneManager_ChangeScene(int32_t nextState);

int SceneManager_GetActiveState(void);
#endif
