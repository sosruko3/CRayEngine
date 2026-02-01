#ifndef MENU_H
#define MENU_H

// Forward declaration
typedef struct EntityRegistry EntityRegistry;
typedef struct CommandBus CommandBus;

void Menu_Init(EntityRegistry* reg, CommandBus* bus);
void Menu_Update(EntityRegistry* reg, CommandBus* bus,float dt);
void Menu_Draw(EntityRegistry* reg, CommandBus* bus);
void Menu_Unload(EntityRegistry* reg, CommandBus* bus);

#endif
