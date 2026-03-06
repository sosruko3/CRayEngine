#include "cre_renderAPI.h"
#include "engine/core/cre_commandBus.h"
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

void renderAPI_SetSprite(CommandBus* restrict bus, Entity entity, uint16_t spriteID) {
    assert(bus != NULL);
    Command cmd = {
        .type = CMD_RENDER_SET_SPRITE,
        .entity = entity,
        .u16 = { 
            .value = spriteID 
        }
    };
    CommandBus_Push(bus, cmd);
}

void renderAPI_SetColor(CommandBus* restrict bus, Entity entity, creColor color) {
    assert(bus != NULL);
    Command cmd = {
        .type = CMD_RENDER_SET_COLOR,
        .entity = entity,
        .color = { 
            .color = color 
        }
    };
    CommandBus_Push(bus, cmd);
}

void renderAPI_SetVisible(CommandBus* restrict bus, Entity entity, bool visible) {
    assert(bus != NULL);
    Command cmd = {
        .type = CMD_RENDER_SET_VISIBLE,
        .entity = entity,
        .b8 = { 
            .value = visible 
        }
    };
    CommandBus_Push(bus, cmd);
}

void renderAPI_SetVisualScale(CommandBus* restrict bus, Entity entity, float scaleX, float scaleY) {
    assert(bus != NULL);
    Command cmd = {
        .type = CMD_RENDER_SET_VISUAL_SCALE,
        .entity = entity,
        .vec2 = { 
            .value = (creVec2){scaleX, scaleY } 
        }
    };
    CommandBus_Push(bus, cmd);
}

void renderAPI_SetRotation(CommandBus* restrict bus, Entity entity, float rotation) {
    assert(bus != NULL);
    Command cmd = {
        .type = CMD_RENDER_SET_ROTATION,
        .entity = entity,
        .f32 = { 
            .value = rotation 
        }
    };
    CommandBus_Push(bus, cmd);
}

void renderAPI_SetRenderLayer(CommandBus* restrict bus, Entity entity, uint8_t layer) {
    assert(bus != NULL);
    Command cmd = {
        .type = CMD_RENDER_SET_LAYER,
        .entity = entity,
        .u8 = { 
            .value = layer 
        }
    };
    CommandBus_Push(bus, cmd);
}

