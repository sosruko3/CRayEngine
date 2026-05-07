#ifndef ASSETSTRUCTS_H
#define ASSETSTRUCTS_H

#include "cre_types.h"
#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>
#include <atomic>
// Compile-time is not important aspect for this tool 
// that is why I am just adding these libraries here.,

inline constexpr size_t MAX_ASSET_RAM_BYTES = 2147483648; // 2 GB hard limit

struct PixelSettings {
    int desired_channels = 4; // 4 means RGBA, 3 means RGB
    bool premultiply_alpha = false; // Important for 2D sprites, 
                                    // be careful if art tool already handles this.
};

struct PCMSettings {
    uint32_t sample_rate = 44100;   // Default
    uint32_t channels = 2;          // Stereo
    uint32_t bit_depth = 32;
};

struct imageItem {
    std::filesystem::path file_path;
    std::string asset_name;

    uint32_t width    = 0;
    uint32_t height   = 0;
    uint32_t channels = 0;
    uint32_t atlas_index = 0;
    creVec2 offset = creVec2{0,0};
    creVec2 atlas_pos = creVec2{0,0};
    uint8_t* pixels = nullptr;
};

struct audioItem {
    std::filesystem::path file_path;
    std::string asset_name;
    uint32_t sample_rate = 0;
    uint32_t channels = 0;
    uint32_t bit_depth = 0;
    uint64_t total_frames = 0;
    uint8_t* pcm_data = nullptr;
};

struct AssetContext {
    std::vector<imageItem> textures;
    std::vector<imageItem> fonts;
    std::vector<audioItem> audios;
    std::atomic<size_t> total_allocated_bytes = {};
};

struct AtlasResult {
    uint8_t* pixels = nullptr;
    uint32_t size = 0; 
    uint32_t channels = 0;
};

#endif