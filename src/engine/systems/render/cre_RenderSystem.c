#include "cre_RenderSystem.h"
#include "cre_RendererCore.h"
#include "engine/ecs/cre_entityManager.h"
#include "engine/ecs/cre_entityRegistry.h"
#include "../physics/cre_spatialHash.h"
#include "engine/core/cre_config.h"
#include "engine/core/cre_typesMacro.h"
#include "atlas_data.h" 
#include <assert.h>

static void _Renderer_SyncDecorations(EntityRegistry* reg) {
    const uint32_t bound = reg->max_used_bound;
    const uint64_t reqComps = (COMP_PHYSICS | COMP_SPRITE);
    
    for (uint32_t i = 0; i <= bound; i++) {
        const uint64_t flags = reg->state_flags[i];
        const uint64_t comps = reg->component_masks[i];
        
        if (!(flags & FLAG_ACTIVE)) continue;
        if (comps & reqComps) continue;

        SpatialHash_AddDynamic(
            i,
            (int)reg->pos_x[i],
            (int)reg->pos_y[i],
            (int)reg->size_w[i],
            (int)reg->size_h[i]
        );
    }
}

void RenderSystem_DrawEntities(EntityRegistry* reg, creRectangle cullRect) {
    assert(reg && "reg is NULL");
    
    static uint32_t visibleEntities[MAX_VISIBLE_ENTITIES];
    
    int visibleCount = SpatialHash_Query(
        (int)cullRect.x, 
        (int)cullRect.y, 
        (int)cullRect.width, 
        (int)cullRect.height, 
        visibleEntities, 
        MAX_VISIBLE_ENTITIES
    );
    
    for (int i = 0; i < visibleCount; i++) {
        uint32_t id = visibleEntities[i];
        
        // Double checking - entity must be active and visible
        uint64_t flags = reg->state_flags[id];
        if ((flags & (FLAG_ACTIVE | FLAG_VISIBLE)) == (FLAG_ACTIVE | FLAG_VISIBLE)) {
            uint16_t spriteIds = reg->sprite_ids[id];
            creVec2 position = {reg->pos_x[id], reg->pos_y[id]};
            creVec2 size = {reg->size_w[id], reg->size_h[id]};
            float rotation = reg->rotation[id];
            float pivotX = reg->pivot_x[id];
            float pivotY = reg->pivot_y[id];
            creColor color = reg->colors[id];
             
            // Entity position is top-left of collision box
            position.x += size.x * pivotX;
            position.y += size.y * pivotY;
            
            // Animation flip data - check if entity has animation component
            bool flipX = false;
            bool flipY = false;
            //if (reg->component_masks[id] & COMP_ANIMATION) {
                // TODO: Add flip data to registry if needed
                // For now, no flip
            //}
            
            RendererCore_DrawSprite(
                spriteIds,
                position,
                size,
                (creVec2){pivotX,pivotY},
                rotation,
                flipX,
                flipY,
                color
            );
        }
    }
}

void RendererSystem_Draw(EntityRegistry* reg,creRectangle view) {
    _Renderer_SyncDecorations(reg);

    // Sorting is required here.
    RenderSystem_DrawEntities(reg, view);
}
