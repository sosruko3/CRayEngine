#ifndef ENGINE_H
#define ENGINE_H

// Forward Declaration
typedef struct EntityRegistry EntityRegistry;
typedef struct CommandBus CommandBus;

void Engine_Init(EntityRegistry* reg, CommandBus* bus,const char* title, const char* configFileName);

void Engine_Run(EntityRegistry* reg, CommandBus* bus,float dt);

void Engine_Shutdown(EntityRegistry* reg, CommandBus* bus);

#endif 