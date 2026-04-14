#ifndef CRE_ENGINE_H
#define CRE_ENGINE_H

// Forward Declaration
struct EngineContext;

void Engine_Init(EngineContext& ctx, const char *title,
                 const char *configFileName);
void Engine_Run(EngineContext& ctx);
void Engine_Shutdown(EngineContext& ctx);

#endif
