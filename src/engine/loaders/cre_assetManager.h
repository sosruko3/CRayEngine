#ifndef CRE_ASSETMANAGER_H
#define CRE_ASSETMANAGER_H

#include "raylib.h"
#include "engine/core/cre_types.h"

void Asset_Init(void);
void Asset_Shutdown(void);
Texture2D Asset_getTexture(void);
creRectangle Asset_getRect(int spriteID);

#endif