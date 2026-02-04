#ifndef CRE_RENDERSYSTEM_H
#define CRE_RENDERSYSTEM_H

#include "raylib.h"
#include "cre_types.h"
typedef struct EntityRegistry EntityRegistry;

/**
 * @brief Draw all visible entities within the cull rectangle.
 * 
 * Uses spatial hash query to find entities in view, then draws them
 * using cre_RendererCore_DrawSprite.
 * 
 * @param reg Pointer to the EntityRegistry
 * @param cullRect The creRectangle defining the visible area (camera bounds)
 */
void cre_RenderSystem_DrawEntities(EntityRegistry* reg, creRectangle cullRect);

#endif