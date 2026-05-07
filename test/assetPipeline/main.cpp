#include "assetStructs.h"
#include "fileScanner.h"
#include "pixelLoader.h"
#include "pcmLoader.h"
#include "atlasBuilder.h"
#include "assetUtils.h"

int main(int argc, char* argv[]) {
    // Using default settings 
    AssetContext ctx = {};
    PixelSettings textureSettings = {};
    PixelSettings fontSettings = {};
    PCMSettings audioSettings = {};

    // 1st phase, get Folder parameter and filter files.
    ScanRawAssets(&ctx,argv[1]);

    // 1.5th phase, Sort all assets according to their main folder, ui/a.png etc.
    // so that all required data is close to each other. Great for atlases and other stuff.
    SortAllAssets(ctx.textures,ctx.audios,ctx.fonts);

    // 2th phase, we decode image files with stb_image, audio files with miniaudio.
    LoadPixels(ctx.textures, ctx.total_allocated_bytes, textureSettings);
    LoadPixels(ctx.fonts, ctx.total_allocated_bytes, fontSettings);
    LoadPCM(ctx.audios, ctx.total_allocated_bytes, audioSettings);

    // 3rd phase, Creating atlases.
    AtlasResult textureAtlas = GenerateTextureAtlas(ctx.textures);

    // 4th phase, combining textureAtlases to one.

    // 5th phase, creating TOC(Table of Contents) for assets.
    
    // 6th phase, write to binary file.

    // Last phase, Freeing the memory.
    FreePixels(ctx.textures);
    FreePixels(ctx.fonts);
    FreePCM(ctx.audios);
    return 0;
}