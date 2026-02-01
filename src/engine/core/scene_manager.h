#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include "scenes.h"

// Forward declarations (cleaner than including headers)
typedef struct EntityRegistry EntityRegistry;
typedef struct CommandBus CommandBus;

typedef Scene (*SceneFactory)(int stateID);

// Start
void SceneManager_Init(SceneFactory factory);

// Main loop calls these
void SceneManager_Update(EntityRegistry* reg, CommandBus* bus, float dt);
void SceneManager_Draw(EntityRegistry* reg, CommandBus* bus);

// Clean up whatever scene is currently active
void SceneManager_Shutdown(EntityRegistry* reg, CommandBus* bus);

// Switch scenes
void SceneManager_ChangeScene(int nextState);

int SceneManager_GetActiveState(void);
#endif
