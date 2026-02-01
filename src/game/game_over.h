#ifndef GAME_OVER_H
#define GAME_OVER_H

// Forward declaration
typedef struct EntityRegistry EntityRegistry;
typedef struct CommandBus CommandBus;

void GameOver_Init(EntityRegistry* reg, CommandBus* bus);
void GameOver_Update(EntityRegistry* reg, CommandBus* bus,float dt);
void GameOver_Draw(EntityRegistry* reg, CommandBus* bus);
void GameOver_Unload(EntityRegistry* reg, CommandBus* bus);

#endif