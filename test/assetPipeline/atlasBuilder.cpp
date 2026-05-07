#include "assetStructs.h"
#include "external/stb_rect_pack.h"
#include <cstdint>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <cstdio>

constexpr uint32_t padding = 2;
constexpr int MAX_ATLAS_SIZE = 4096;

AtlasResult PackSingleAtlas(std::vector<imageItem>& items, size_t start_idx, size_t end_idx, uint32_t current_atlas_index) {
    
    size_t chunk_size = end_idx - start_idx;
    uint32_t expected_channels = items[start_idx].channels;

    // Safety Check: Ensure all images in this atlas chunk share the same channel count
    for (size_t i = start_idx; i < end_idx; i++) {
        if (items[i].channels != expected_channels) {
            fprintf(stderr, "ERROR: Mixed channel counts in the same atlas folder!\n");
            fprintf(stderr, "File '%s' has %u channels, expected %u.\n", items[i].file_path.string().c_str(), items[i].channels, expected_channels);
            fprintf(stderr, "Tool execution aborted.\n");
            exit(EXIT_FAILURE);
        }
    }

    std::vector<stbrp_rect> rects(chunk_size);

    // Create RECTS
    for (size_t i = 0; i < chunk_size; i++) {
        size_t actual_idx = start_idx + i;
        
        rects[i].id = static_cast<int>(actual_idx);
        rects[i].w  = static_cast<int>(items[actual_idx].width + padding);   
        rects[i].h  = static_cast<int>(items[actual_idx].height + padding);  
    }

    // 2. DYNAMIC SIZE FINDER 
    int atlas_size = 512;
    bool all_packed = false;
    std::vector<stbrp_node> nodes;
    
    while (!all_packed) {
        nodes.resize(static_cast<size_t>(atlas_size)); 
        stbrp_context pack_ctx = {};
        stbrp_init_target(&pack_ctx, atlas_size, atlas_size, nodes.data(), atlas_size);
                          
        stbrp_pack_rects(&pack_ctx, rects.data(), static_cast<int>(rects.size()));

        all_packed = true;
        for (size_t i = 0; i < rects.size(); i++) {
            if (rects[i].was_packed == 0) {
                all_packed = false;
                break;
            }
        }

        if (!all_packed) {
            if (atlas_size >= MAX_ATLAS_SIZE) {
                fprintf(stderr, "ERROR: Could not fit textures into max atlas size %d!\n", MAX_ATLAS_SIZE);
                fprintf(stderr, "Tool execution aborted.\n");
                exit(EXIT_FAILURE);
            }
            atlas_size *= 2; 
        }
    }

    // CALLOC
    AtlasResult result = {};
    result.size = static_cast<uint32_t>(atlas_size);
    result.channels = items[start_idx].channels; 
    result.pixels = static_cast<uint8_t*>(calloc(static_cast<size_t>(atlas_size * atlas_size * result.channels), 1));
    
    if (result.pixels == nullptr) {
        result.size = 0;
        fprintf(stderr, "ERROR: Failed to allocate memory for texture atlas\n");
        fprintf(stderr, "Tool execution aborted.\n");
        exit(EXIT_FAILURE);
    }

    // Copy required data 
    for (size_t i = 0; i < rects.size(); i++) {
        imageItem& tex = items[static_cast<size_t>(rects[i].id)]; 

        uint32_t start_x = static_cast<uint32_t>(rects[i].x + 1);
        uint32_t start_y = static_cast<uint32_t>(rects[i].y + 1);

        tex.atlas_pos = creVec2{ static_cast<float>(start_x), static_cast<float>(start_y) };
        tex.atlas_index = current_atlas_index;

        for (uint32_t row = 0; row < tex.height; row++) {
            uint8_t* src_row = tex.pixels + (row * tex.width * result.channels);
            
            uint32_t dest_x = start_x;
            uint32_t dest_y = start_y + row;
            uint8_t* dest_row = result.pixels + ((dest_y * result.size + dest_x) * result.channels);
            
            memcpy(dest_row, src_row, tex.width * result.channels);
        }

        // Free pixels
        if (tex.pixels != nullptr) {
            free(tex.pixels);
            tex.pixels = nullptr; 
        }
    }

    return result;
}

std::vector<AtlasResult> GenerateTextureAtlases(std::vector<imageItem>& items) {
    std::vector<AtlasResult> all_atlases;
    if (items.empty()) return all_atlases;

    size_t current_idx = 0;
    uint32_t current_atlas_index = 0;

    while (current_idx < items.size()) {
        
        const std::filesystem::path& current_folder = items[current_idx].file_path.parent_path();
        
        size_t chunk_end = current_idx + 1;
        while (chunk_end < items.size() && items[chunk_end].file_path.parent_path() == current_folder) {
            chunk_end++;
        }

        AtlasResult result = PackSingleAtlas(items, current_idx, chunk_end, current_atlas_index);
        all_atlases.push_back(result);

        current_idx = chunk_end;
        current_atlas_index++;
    }
    return all_atlases;
}