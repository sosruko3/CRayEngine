#include "fileScanner.h"
#include "assetStructs.h"
#include <iterator> 
#include <string>
#include <system_error>

namespace fs = std::filesystem;

// NOTE: Tool assumes extensions are lower case, do not use something like ASSET.PNG
// NOTE: Tool assumes filenames are english. No ü,ç,ş, etc.
typedef void (*RegisterFunc)(AssetContext* ctx, const fs::path& path);

void RegisterTexture(AssetContext* ctx, const fs::path& path) {
    imageItem texture = {
        .file_path = path,
        .asset_name = path.stem().string()
    };
    ctx->textures.push_back(texture);
}

void RegisterFont(AssetContext* ctx, const fs::path& path) {
    imageItem font = {
        .file_path = path,
        .asset_name = path.stem().string()
    };
    ctx->fonts.push_back(font);
}

void RegisterAudio(AssetContext* ctx, const fs::path& path) {
    audioItem audio = {
        .file_path = path,
        .asset_name = path.stem().string()
    };
    ctx->audios.push_back(audio);
}

struct DispatchEntry {
    const char* suffix;
    RegisterFunc registerIT;
};

const DispatchEntry DispatchTable[] = {
    {"_msdf.png", RegisterFont}, 
    {"_font.png", RegisterFont},

    {".png",  RegisterTexture},
    {".jpg",  RegisterTexture},
    {".jpeg", RegisterTexture},
    {".tga",  RegisterTexture},

    {".wav",  RegisterAudio},
    {".mp3",  RegisterAudio},
    {".ogg",  RegisterAudio}
};

void ScanRawAssets(AssetContext* ctx, const std::string& folder_path) {
    std::filesystem::directory_options options = fs::directory_options::skip_permission_denied;
    std::error_code error_code;
    
    for (const fs::directory_entry& entry : fs::recursive_directory_iterator(folder_path,options)) {
        
        if (!entry.is_regular_file(error_code) || error_code) {
            error_code.clear();
            continue;
        } 
        std::string filename = entry.path().filename().string();

        for (size_t i = 0; i < std::size(DispatchTable); i++) {
            if (filename.ends_with(DispatchTable[i].suffix)) {
                DispatchTable[i].registerIT(ctx, entry.path());
                break; 
            }
        }
    }
}