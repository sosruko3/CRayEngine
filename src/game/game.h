#ifndef GAME_H
#define GAME_H

// Forward declaration
typedef struct EntityRegistry EntityRegistry;
typedef struct CommandBus CommandBus;

void Game_Init(EntityRegistry* reg, CommandBus* bus);
void Game_Update(EntityRegistry* reg, CommandBus* bus,float dt);
void Game_Draw(EntityRegistry* reg, CommandBus* bus);
void Game_Shutdown(EntityRegistry* reg, CommandBus* bus);

#endif