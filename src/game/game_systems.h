#ifndef GAME_SYSTEMS_H
#define GAME_SYSTEMS_H

#include <stdint.h>
#include "raylib.h"

void System_UpdateLogic(float dt);
void System_HandleDebugInput(void);
void System_DrawEntities(Rectangle cullRect);
void SystemTestSpawn(void);
int GetActiveEntityCount(void);
void SpawnPlayer(void);
void System_SetCameraTarget(uint32_t entityID);
void System_UpdateCamera(void);
void System_UpdateSleepState(void);
void System_DrawDebug(void);
void System_ChangeZoom(void);
#endif