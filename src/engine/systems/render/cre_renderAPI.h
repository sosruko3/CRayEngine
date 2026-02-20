#ifndef CRE_RENDERAPI_H
#define CRE_RENDERAPI_H


#include <stdint.h>
typedef struct CommandBus CommandBus;

typedef enum {
    DEPTH_PRESET_FLAT,
    DEPTH_PRESET_Y_ORIGIN,
    DEPTH_PRESET_Y_BOTTOM,
    DEPTH_PRESET_ISOMETRIC
} creDepthPreset;

void renderAPI_SetDepthSettings(CommandBus* restrict bus,float wX, float wY, float wH, uint8_t shiftBatch, uint8_t shiftDepth);

void renderAPI_SetDepthPreset(CommandBus* restrict bus, creDepthPreset type);
#endif
