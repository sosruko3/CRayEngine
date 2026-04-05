#include "cre_assetManager.h"
#include "atlas_data.h"
#include "engine/core/cre_logger.h"
#include "engine/core/cre_types.h"
#include "raylib.h"
// [NOTE] Fix naming inconsistencies in this file.
// use "assetManager_" prefix.
static Texture2D atlasTexture;

// Helper: Convert SpriteMeta to creRectangle for rendering
static inline creRectangle SpriteMeta_ToRect(const SpriteMeta *meta) {
  return creRectangle{static_cast<float>(meta->x), static_cast<float>(meta->y),
                      static_cast<float>(meta->w), static_cast<float>(meta->h)};
}

void Asset_Init(void) {
  // Try multiple paths to find atlas.png
  // 1. Relative to executable (build directory)
  const char *exeDir = GetApplicationDirectory();
  const char *atlasPath = TextFormat("%satlas.png", exeDir);

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
    Log(LogLevel::Info, "ASSETS: Atlas loaded successfully.");
  } else {
    Log(LogLevel::Error, "ASSETS: Failed to load atlas.png!");
  }
}
void Asset_Shutdown(void) {
  UnloadTexture(atlasTexture);
  Log(LogLevel::Info, "ASSETS: Atlas unloaded succesfully.");
}
Texture2D Asset_getTexture(void) { return atlasTexture; }
// why does this parameter has int? Change this to uint32_t after changing
// spriteids to uint32_t
creRectangle Asset_getRect(int spriteID) {
  if (spriteID < 0 || spriteID >= SPRITE_COUNT) {
    Log(LogLevel::Warning, "ASSETS: Missing sprite ID {},using Fallback.",
        spriteID);
    return SpriteMeta_ToRect(&ASSET_SPRITES[SPR_MISSING]);
  }
  return SpriteMeta_ToRect(&ASSET_SPRITES[spriteID]);
}
