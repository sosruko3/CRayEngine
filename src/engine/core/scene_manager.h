#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include "game_types.h"

// Start
void SceneManager_Init(void);

// Main loop calls these
void SceneManager_Update(void);
void SceneManager_Draw(void);

// Clean up whatever scene is currently active
void SceneManager_Shutdown(void);

// Switch scenes
void SceneManager_ChangeScene(GameState nextState);

#endif
