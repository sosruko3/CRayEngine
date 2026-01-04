#ifndef ENGINE_H
#define ENGINE_H


void Engine_Init(int width, int height, const char* title, const char* configFileName);

void Engine_Run(void);

void Engine_Shutdown(void);

#endif // ENGINE_H