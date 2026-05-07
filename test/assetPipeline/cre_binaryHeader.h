#ifndef CRE_BINARYHEADER_H
#define CRE_BINARYHEADER_H

#include <stdint.h>


#pragma pack(push, 1)
struct creHeader{
    uint32_t signature;
    uint32_t version;
    uint32_t asset_type;
    uint32_t data_size;
    uint32_t asset_count;
    uint64_t flags;

    union {
        // image
        struct {
            uint16_t width;
            uint16_t height;
            uint8_t  channels;
            uint8_t  padding[3];
        }image;

        // audio
        struct {
            uint32_t sample_rate;
            uint8_t  channels;
            uint8_t  bit_depth;
            uint8_t  padding[2];
        }audio;
    }; 
};
#pragma pack(pop)

#endif
