#include "cre_renderSystem.h"
#include "cre_rendererCore.h"
#include "engine/ecs/cre_entityRegistry.h"
#include "engine/systems/physics/cre_spatialHash.h"
#include "engine/loaders/cre_assetManager.h"
#include "engine/core/cre_commandBus.h"
#include "engine/core/cre_config.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

typedef uint64_t SortKey;

typedef struct RenderState {
    Texture2D* texture;
    Shader* shader;
    int32_t blendMode;
    int32_t filterMode;
} RenderState;

static RenderState render_state_table[256];
static Texture2D s_defaultTexture;
static bool s_batchTableInitialized = false;

static uint8_t g_shiftDepth = 24;
static uint8_t g_shiftBatch = 48;
static uint8_t g_shiftLayer = 56;

static float g_WeightX = 0.0f;
static float g_WeightY = 0.0f;
static float g_WeightH = 0.0f;

#define SORT_BITS_LAYER   8
#define SORT_BITS_BATCH   8
#define SORT_BITS_DEPTH   24
#define SORT_BITS_ID      24

#define SORT_MASK_LAYER   ((1ULL << SORT_BITS_LAYER) - 1)   // 0xFF
#define SORT_MASK_BATCH   ((1ULL << SORT_BITS_BATCH) - 1)   // 0xFF
#define SORT_MASK_DEPTH   ((1ULL << SORT_BITS_DEPTH) - 1)   // 0xFFFFFF
#define SORT_MASK_ID      ((1ULL << SORT_BITS_ID) - 1)      // 0xFFFFFF

// Depth settings
#define SORT_DEPTH_BIAS       100000.0f
#define SORT_DEPTH_PRECISION  100.0f
#define SORT_DEPTH_MAX        (SORT_MASK_DEPTH)

void renderSystem_SetDepthMath(float wX, float wY, float wH, uint8_t shiftBatch, uint8_t shiftDepth) {
    assert(isfinite(wX) && isfinite(wY) && isfinite(wH));

    assert(shiftDepth < 64U);
    assert(shiftBatch < 64U);
    assert((uint32_t)shiftDepth + SORT_BITS_DEPTH <= 64U);
    assert((uint32_t)shiftBatch + SORT_BITS_BATCH <= 64U);

    const uint64_t idMask = SORT_MASK_ID;
    const uint64_t depthMask = SORT_MASK_DEPTH << shiftDepth;
    const uint64_t batchMask = SORT_MASK_BATCH << shiftBatch;
    const uint64_t layerMask = SORT_MASK_LAYER << g_shiftLayer;

    assert((depthMask & idMask) == 0ULL);
    assert((batchMask & idMask) == 0ULL);
    assert((layerMask & idMask) == 0ULL);

    assert((depthMask & batchMask) == 0ULL);
    assert((depthMask & layerMask) == 0ULL);
    assert((batchMask & layerMask) == 0ULL);

    g_WeightX = wX;
    g_WeightY = wY;
    g_WeightH = wH;
    
    g_shiftBatch = shiftBatch;
    g_shiftDepth = shiftDepth;
}

void renderSystem_ProcessCommands(EntityRegistry* reg, const CommandBus* bus) {
    if (!reg || !bus) return;

    CommandIterator iter = CommandBus_GetIterator(bus);
    const Command* cmd;
        
    while (CommandBus_Next(bus, &iter, &cmd)) {
        if ((cmd->type & CMD_DOMAIN_MASK) != CMD_DOMAIN_RENDER) continue;

        switch (cmd->type) {
            case CMD_RENDER_SETDEPTHMATH: {
                renderSystem_SetDepthMath(
                    cmd->renderDepth.wX,
                    cmd->renderDepth.wY,
                    cmd->renderDepth.wH,
                    cmd->renderDepth.shiftBatch,
                    cmd->renderDepth.shiftDepth
                );
                break;
            }
            
            default:
                break;
        }
    }
}

static inline SortKey PackSortKey(uint8_t layer, uint8_t batchID, uint32_t depth, uint32_t entityID) {
    const uint64_t layerPart = (((uint64_t)layer) & SORT_MASK_LAYER) << g_shiftLayer;
    const uint64_t batchPart = (((uint64_t)batchID) & SORT_MASK_BATCH) << g_shiftBatch;
    const uint64_t depthPart = (((uint64_t)depth) & SORT_MASK_DEPTH) << g_shiftDepth;
    const uint64_t entityPart = ((uint64_t)entityID) & SORT_MASK_ID;
    return (SortKey)(layerPart | batchPart | depthPart | entityPart);
}

static inline uint32_t UnpackEntityID(SortKey key) {
    return (uint32_t)(key & SORT_MASK_ID);
}

static inline uint8_t UnpackBatchID(SortKey key) {
    return (uint8_t)((key >> g_shiftBatch) & SORT_MASK_BATCH);
}

static inline uint32_t QuantizeDepth(float y) {
    const float quantized = (y + SORT_DEPTH_BIAS) * SORT_DEPTH_PRECISION;
    if (quantized <= 0.0f) return 0U;
    if (quantized >= (float)SORT_DEPTH_MAX) return (uint32_t)SORT_DEPTH_MAX;
    return (uint32_t)quantized;
}

static int SortKeyCompare(const void* lhs, const void* rhs) {
    const SortKey a = *(const SortKey*)lhs;
    const SortKey b = *(const SortKey*)rhs;
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

void renderSystem_RegisterBatch(uint8_t id, Texture2D* tex, Shader* shd, int32_t blend, int32_t filterMode) {
    render_state_table[id] = (RenderState){
        .texture = tex,
        .shader = shd,
        .blendMode = blend,
        .filterMode = filterMode
    };
}

static void _renderSystem_InitBatchTable(void) {
    if (s_batchTableInitialized) return;

    memset(render_state_table, 0, sizeof(render_state_table));

    // You can add this as an parameter. No need to link assetManager for one line.
    s_defaultTexture = Asset_getTexture();

    renderSystem_RegisterBatch(RENDER_BATCH_DEFAULT, &s_defaultTexture, NULL, BLEND_ALPHA, TEXTURE_FILTER_POINT);
    renderSystem_RegisterBatch(RENDER_BATCH_PLAYER, &s_defaultTexture, NULL, BLEND_ALPHA, TEXTURE_FILTER_POINT);
    renderSystem_RegisterBatch(RENDER_BATCH_ENEMY, &s_defaultTexture, NULL, BLEND_ALPHA, TEXTURE_FILTER_POINT);

    s_batchTableInitialized = true;
}

static void _Renderer_SyncDecorations(EntityRegistry* reg) {
    // IMPORTANT:Move this funtion to somewhere else!!!
    const uint32_t bound = reg->max_used_bound;
    const uint64_t reqComps = (COMP_SPRITE);
    const uint64_t notreqComps = (COMP_PHYSICS);
    
    for (uint32_t i = 0; i <= bound; i++) {
        const uint64_t flags = reg->state_flags[i];
        const uint64_t comps = reg->component_masks[i];
        
        if ((flags & (FLAG_ACTIVE | FLAG_STATIC)) != (FLAG_ACTIVE | FLAG_STATIC)) continue;
        if ((!(comps & reqComps)) || (comps & notreqComps)) continue;
        // Add to static hash if have active+static+sprite but not physics.

        SpatialHash_AddStatic(
            i,
            (int)reg->pos_x[i],
            (int)reg->pos_y[i],
            (int)reg->size_w[i],
            (int)reg->size_h[i]
        );
    }
}

void renderSystem_DrawEntities(EntityRegistry* reg, creRectangle cullRect) {
    assert(reg && "reg is NULL");
    _renderSystem_InitBatchTable();
    
    static uint32_t visibleEntities[MAX_VISIBLE_ENTITIES];
    static SortKey sortKeys[MAX_VISIBLE_ENTITIES];
    
    const int visibleCount = SpatialHash_Query(
        (int)cullRect.x, 
        (int)cullRect.y, 
        (int)cullRect.width, 
        (int)cullRect.height, 
        visibleEntities, 
        MAX_VISIBLE_ENTITIES
    );

    int sortCount = 0;
    for (int i = 0; i < visibleCount; i++) {
        const uint32_t id = visibleEntities[i];
        if (id >= MAX_ENTITIES) continue;

        const uint64_t flags = reg->state_flags[id];
        if ((flags & (FLAG_ACTIVE | FLAG_VISIBLE)) != (FLAG_ACTIVE | FLAG_VISIBLE)) continue;

        const float rawDepth = (reg->pos_x[id] * g_WeightX) +
                               (reg->pos_y[id] * g_WeightY) +
                               (reg->size_h[id] * g_WeightH);
        const uint32_t depth = QuantizeDepth(rawDepth);
        const uint8_t layer = reg->render_layer[id];
        const uint8_t batchID = reg->batch_ids[id];

        sortKeys[sortCount++] = PackSortKey(layer, batchID, depth, id);
    }

    qsort(sortKeys, (size_t)sortCount, sizeof(SortKey), SortKeyCompare);

    int16_t lastBatch = -1;
    for (int i = 0; i < sortCount; i++) {
        const SortKey key = sortKeys[i];
        const uint32_t id = UnpackEntityID(key);
        const uint8_t currentBatch = UnpackBatchID(key);

        if (currentBatch != lastBatch) {
            const RenderState rs = render_state_table[currentBatch];
            rendererCore_SetState(rs.texture, rs.shader, rs.blendMode, rs.filterMode);
            lastBatch = currentBatch;
        }

        const uint16_t spriteID = reg->sprite_ids[id];
        creVec2 position = {reg->pos_x[id], reg->pos_y[id]};
        const creVec2 size = {reg->size_w[id], reg->size_h[id]};
        const float rotation = reg->rotation[id];
        const float pivotX = reg->pivot_x[id];
        const float pivotY = reg->pivot_y[id];
        const creColor color = reg->colors[id];
        const bool flipX = false;
        const bool flipY = false;

        //position.x += size.x * pivotX;
        //position.y += size.y * pivotY;

        rendererCore_DrawSprite(
            spriteID,
            position,
            size,
            (creVec2){pivotX, pivotY},
            rotation,
            flipX,
            flipY,
            color
        );
    }
    rendererCore_EndBatch();
}

void renderSystem_Draw(EntityRegistry* reg,CommandBus* bus,creRectangle view) {
    renderSystem_ProcessCommands(reg,bus);
    _Renderer_SyncDecorations(reg);

    renderSystem_DrawEntities(reg, view);
}
