#ifndef ENGINE_H
#define ENGINE_H

// Define the function pointer types
// "A pointer to a function that takes void and returns void"
typedef void (*GameUpdateCallback)(void);
typedef void (*GameDrawCallback)(void);

void Engine_Init(int width, int height, const char* title);

// Update Run to accept these callbacks
void Engine_Run(GameUpdateCallback update, GameDrawCallback draw);

void Engine_Shutdown(void);

#endif // ENGINE_H