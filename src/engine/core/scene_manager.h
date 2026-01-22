#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include "scenes.h"

typedef Scene (*SceneFactory)(int stateID);

// Start
void SceneManager_Init(SceneFactory factory);

// Main loop calls these
void SceneManager_Update(void);
void SceneManager_Draw(void);

// Clean up whatever scene is currently active
void SceneManager_Shutdown(void);

// Switch scenes
void SceneManager_ChangeScene(int nextState);

int SceneManager_GetActiveState(void);
#endif
