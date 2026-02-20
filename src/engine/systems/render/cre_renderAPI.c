#include "cre_renderAPI.h"
#include "engine/core/cre_commandBus.h"
#include "cre_renderSystem.h"
#include <assert.h>

void renderAPI_SetDepthSettings(
    CommandBus* restrict bus,float wX, float wY, float wH,
     uint8_t shiftBatch, uint8_t shiftDepth) {

    assert(bus != NULL);
    Command cmd = {
        .type = CMD_RENDER_SETDEPTHMATH,
        .entity = ENTITY_INVALID,
        .renderDepth = {
            .wX = wX,
            .wY = wY,
            .wH = wH,
            .shiftBatch = shiftBatch,
            .shiftDepth = shiftDepth
        }
    };
    CommandBus_Push(bus,cmd);
}

void renderAPI_SetDepthPreset(CommandBus* restrict bus, creDepthPreset type) {
    assert(bus != NULL && "CommandBus cannot be NULL");

    switch (type) {
        case DEPTH_PRESET_FLAT:
        // Platformers etc.
            renderAPI_SetDepthSettings(bus, 0.0f, 0.0f, 0.0f, 48, 24);
            break;

        case DEPTH_PRESET_Y_BOTTOM:
        // Like old pokemon/zelda games.
            renderAPI_SetDepthSettings(bus, 0.0f, 1.0f, 1.0f, 24, 32);
            break;

        case DEPTH_PRESET_Y_ORIGIN:
        // Like hotline miami,or gta1.
            renderAPI_SetDepthSettings(bus, 0.0f, 1.0f, 0.0f, 24, 32);
            break;

        case DEPTH_PRESET_ISOMETRIC:
        // Like BG1,BG2,Pillars of eternity.
            renderAPI_SetDepthSettings(bus, 0.5f, 0.5f, 1.0f, 24, 32);
            break;

        default:
            assert(0 && "Unknown creDepthPreset provided to RenderAPI!");
            break;
    }
}   
