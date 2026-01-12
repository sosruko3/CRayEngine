#ifndef GAME_SYSTEMS_H
#define GAME_SYSTEMS_H

void System_UpdateLogic(float dt);
void System_HandleDebugInput(void);
void System_DrawEntities(void);
void SystemTestSpawn(void);
int GetActiveEntityCount(void);

#endif