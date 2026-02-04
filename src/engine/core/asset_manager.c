#include "asset_manager.h"
#include "raylib.h"
#include "logger.h"
#include "config.h"
#include "atlas_data.h"
#include "cre_types.h"

static Texture2D atlasTexture;

// Helper: Convert SpriteMeta to creRectangle for rendering
static inline creRectangle SpriteMeta_ToRect(const SpriteMeta* meta) {
    return (creRectangle){ (float)meta->x, (float)meta->y, (float)meta->w, (float)meta->h };
}

void Asset_Init(void) {
    // Try multiple paths to find atlas.png
    // 1. Relative to executable (build directory)
    const char* exeDir = GetApplicationDirectory();
    const char* atlasPath = TextFormat("%satlas.png", exeDir);
    
    if (FileExists(atlasPath)) {
        atlasTexture = LoadTexture(atlasPath);
    }
    // 2. Current working directory
    else if (FileExists("atlas.png")) {
        atlasTexture = LoadTexture("atlas.png");
    }
    // 3. build/ subdirectory (if running from project root)
    else if (FileExists("build/atlas.png")) {
        atlasTexture = LoadTexture("build/atlas.png");
    }
    
    if (atlasTexture.id) {
        SetTextureFilter(atlasTexture, TEXTURE_FILTER_POINT);
        Log(LOG_LVL_INFO, "ASSETS: Atlas loaded successfully.");
    } else {
        Log(LOG_LVL_ERROR, "ASSETS: Failed to load atlas.png!");
    }
}
void Asset_Shutdown(void) {
    UnloadTexture(atlasTexture);
    Log(LOG_LVL_INFO,"ASSETS: Atlas unloaded succesfully.");
}
Texture2D Asset_getTexture(void) {
    return atlasTexture;
}
creRectangle Asset_getRect(int spriteID) {
    if (spriteID < 0 || spriteID >= SPRITE_COUNT) {
        Log(LOG_LVL_WARNING,"ASSETS: Missing sprite ID %d,using Fallback.",spriteID);
        return SpriteMeta_ToRect(&ASSET_SPRITES[SPR_MISSING]);
    }
    return SpriteMeta_ToRect(&ASSET_SPRITES[spriteID]);
}