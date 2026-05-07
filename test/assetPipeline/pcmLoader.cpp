// pcmLoader.cpp
#include "assetStructs.h"
#include <cstdio>
#include <cstdlib>

#include "external/miniaudio.h"

void LoadPCM(std::vector<audioItem>& items, std::atomic<size_t>& memory_counter, PCMSettings settings) {
    for (size_t i = 0; i < items.size(); i++) {
        
        ma_decoder decoder = {};
        ma_uint64 total_frames = 0;
        ma_uint64 frames_read = 0;
        ma_format target_format = (settings.bit_depth == 32) ? ma_format_f32 : ma_format_s16;
        
        ma_decoder_config config = ma_decoder_config_init(
            target_format, 
            settings.channels, 
            settings.sample_rate
        );
        
        ma_result result = ma_decoder_init_file(
            items[i].file_path.string().c_str(), 
            &config, 
            &decoder
        );
        
        if (result != MA_SUCCESS) {
            fprintf(stderr, "[ERROR] Failed to decode audio: %s\n", items[i].file_path.string().c_str());
            fprintf(stderr, "Tool execution aborted.\n");
            exit(EXIT_FAILURE);
        }
        
        ma_decoder_get_length_in_pcm_frames(&decoder, &total_frames);

        if (total_frames == 0) {
            fprintf(stderr, "[ERROR] Audio file has 0 frames (corrupted/empty): %s\n", items[i].file_path.string().c_str());
            ma_decoder_uninit(&decoder);
            exit(EXIT_FAILURE);
        }
        
        items[i].total_frames = total_frames;
        items[i].sample_rate = settings.sample_rate;
        items[i].channels = settings.channels;
        
        uint32_t bytes_per_sample = settings.bit_depth / 8;
        size_t current_audio_bytes = total_frames * settings.channels * bytes_per_sample;

        size_t new_total = memory_counter.fetch_add(
            current_audio_bytes, std::memory_order_relaxed) + current_audio_bytes;
            
        if (new_total > MAX_ASSET_RAM_BYTES) {
            fprintf(stderr, "[ERROR] RAM limit (2 GB) exceeded at file: %s\n", items[i].asset_name.c_str());
            ma_decoder_uninit(&decoder);
            exit(EXIT_FAILURE);
        }
        
        items[i].pcm_data = static_cast<uint8_t*>(malloc(current_audio_bytes));
        
        ma_decoder_read_pcm_frames(&decoder, items[i].pcm_data, total_frames, &frames_read);
        ma_decoder_uninit(&decoder);
    }
}

void FreePCM(std::vector<audioItem>& items) {
    for (size_t i = 0; i < items.size(); i++) {
        if (items[i].pcm_data) {
            free(items[i].pcm_data);
            items[i].pcm_data = nullptr;
        }
    }
}