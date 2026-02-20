#ifndef CRE_RENDERSYSTEM_H
#define CRE_RENDERSYSTEM_H

#include "engine/core/cre_types.h"
#include "raylib.h"
#include <stdint.h>
typedef struct EntityRegistry EntityRegistry;
typedef struct CommandBus CommandBus;

/**
 * @brief Draw all visible entities within the cull rectangle.
 * 
 * Uses spatial hash query to find entities in view, then draws them
 * using rendererCore_DrawSprite.
 * 
 * @param reg Pointer to the EntityRegistry
 * @param cullRect The creRectangle defining the visible area (camera bounds)
 */
void renderSystem_DrawEntities(EntityRegistry* reg, creRectangle cullRect);

void renderSystem_RegisterBatch(uint8_t id, Texture2D* tex, Shader* shd, int32_t blend, int32_t filterMode);

void renderSystem_Draw(EntityRegistry* reg, CommandBus* bus, creRectangle view);

#endif