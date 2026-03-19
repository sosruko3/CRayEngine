#ifndef CRE_CAMERASYSTEM_H
#define CRE_CAMERASYSTEM_H

#include "engine/core/cre_types.h"
#include "engine/ecs/cre_components.h"
#include "engine/platform/cre_viewport.h"

typedef struct EntityRegistry EntityRegistry;
typedef struct CommandBus CommandBus;

void cameraSystem_Init(EntityRegistry *reg);

void cameraSystem_ProcessCommands(EntityRegistry *reg, CommandBus *bus);
void cameraSystem_Update(EntityRegistry *reg, CommandBus *bus, float dt,
                         ViewportSize vp);

CameraComponent cameraSystem_CreateDefault(void);

creRectangle cameraSystem_GetViewBounds(const EntityRegistry *reg,
                                        const CameraComponent *cam,
                                        ViewportSize vp);
creRectangle cameraSystem_GetCullBounds(const EntityRegistry *reg,
                                        const CameraComponent *cam,
                                        ViewportSize vp);
int32_t cameraSystem_FindActive(const EntityRegistry *reg);

const CameraComponent *
cameraSystem_GetActiveComponent(const EntityRegistry *reg);
creRectangle cameraSystem_GetActiveCullBounds(const EntityRegistry *reg,
                                              const CameraComponent *cam,
                                              ViewportSize vp);

#endif
