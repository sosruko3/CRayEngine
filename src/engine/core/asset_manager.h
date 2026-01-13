#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include "raylib.h"

void Asset_Init(void);
void Asset_Shutdown(void);
Texture2D Asset_getTexture(void);
Rectangle Asset_getRect(int spriteID);

#endif