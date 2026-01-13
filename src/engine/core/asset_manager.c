#include "asset_manager.h"
#include "logger.h"
#include "config.h"
#include "atlas_data.h"

static Texture2D atlasTexture;
void Asset_Init(void) {
    atlasTexture = LoadTexture(atlasDir);
    SetTextureFilter(atlasTexture,TEXTURE_FILTER_POINT);
    if (!atlasTexture.id) Log(LOG_LVL_ERROR,"ASSETS: Failed to load atlas.png!");
    else Log(LOG_LVL_INFO,"ASSETS: Atlas loaded successfully.");
}
void Asset_Shutdown(void) {
    UnloadTexture(atlasTexture);
    Log(LOG_LVL_INFO,"ASSETS: Atlas unloaded succesfully.");
}
Texture2D Asset_getTexture(void) {
    return atlasTexture;
}
Rectangle Asset_getRect(int spriteID) {
    if (spriteID < 0 || spriteID >= SPR_COUNT) {
        Log(LOG_LVL_WARNING,"ASSETS: Missing sprite ID %d,using Fallback.",spriteID);
        return atlas_rects[SPR_missing];
    }
    return atlas_rects[spriteID];

}