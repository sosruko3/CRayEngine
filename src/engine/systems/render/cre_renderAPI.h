#ifndef CRE_RENDERAPI_H
#define CRE_RENDERAPI_H

#include "engine/core/cre_types.h"

#include <stdbool.h>
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
void renderAPI_SetSprite(CommandBus* restrict bus, Entity entity, uint16_t spriteID);
void renderAPI_SetColor(CommandBus* restrict bus, Entity entity, creColor color);
void renderAPI_SetVisible(CommandBus* restrict bus, Entity entity, bool visible);
void renderAPI_SetVisualScale(CommandBus* restrict bus, Entity entity, float scaleX, float scaleY);
void renderAPI_SetRotation(CommandBus* restrict bus, Entity entity, float rotation);
void renderAPI_SetRenderLayer(CommandBus* restrict bus, Entity entity, uint8_t layer);
#endif
