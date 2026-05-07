#include "assetStructs.h"
#include "external/stb_image.h"
#include <cstdio>  
#include <cstdlib> 

void LoadPixels(std::vector<imageItem>& items,std::atomic<size_t>& memory_counter, PixelSettings settings) {
    for (size_t i = 0; i < items.size(); i++) {
        int w, h, c;
        items[i].pixels = stbi_load(
            items[i].file_path.string().c_str(), 
            &w, &h, &c, 
            settings.desired_channels
        );
        
        if (!items[i].pixels) {
            fprintf(stderr, "[ERROR] Failed to decode image: %s\n", 
                items[i].file_path.string().c_str());
            fprintf(stderr, "Tool execution aborted.\n");
            exit(EXIT_FAILURE);
        }
        
        items[i].width = static_cast<uint32_t>(w);
        items[i].height = static_cast<uint32_t>(h);
        items[i].channels = static_cast<uint32_t>(settings.desired_channels);
        
        
        // Thread-safe lock-free addition
        size_t current_image_bytes = items[i].width * items[i].height * items[i].channels;
        size_t new_total = memory_counter.fetch_add(
            current_image_bytes, std::memory_order_relaxed) + current_image_bytes;
        
        if (new_total > MAX_ASSET_RAM_BYTES) {
            fprintf(stderr, "[ERROR] RAM limit (2 GB) exceeded at file: %s\n", 
                items[i].asset_name.c_str());
            fprintf(stderr, "Tool execution aborted.\n");
            exit(EXIT_FAILURE);
        }
        
        if (settings.premultiply_alpha && settings.desired_channels == 4) {
            for (size_t p = 0; p < current_image_bytes; p += 4) {
                uint8_t alpha = items[i].pixels[p + 3];
                items[i].pixels[p + 0] = static_cast<uint8_t>((items[i].pixels[p + 0] * alpha) / 255); // R
                items[i].pixels[p + 1] = static_cast<uint8_t>((items[i].pixels[p + 1] * alpha) / 255); // G
                items[i].pixels[p + 2] = static_cast<uint8_t>((items[i].pixels[p + 2] * alpha) / 255); // B
            }
        }
    }
}

void FreePixels(std::vector<imageItem>& items) {
    for (size_t i = 0; i < items.size(); i++) {
        if (items[i].pixels) {
            stbi_image_free(items[i].pixels);
            items[i].pixels = nullptr;
        }
    }
}