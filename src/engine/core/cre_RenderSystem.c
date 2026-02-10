#include "cre_RenderSystem.h"
#include "cre_RendererCore.h"
#include "entity_manager.h"
#include "entity_registry.h"
#include "../physics/spatial_hash.h"
#include "config.h"
#include "types_macro.h"
#include "atlas_data.h" 
#include <assert.h>

void cre_RenderSystem_DrawEntities(EntityRegistry* reg, creRectangle cullRect) {
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
            if (reg->component_masks[id] & COMP_ANIMATION) {
                // TODO: Add flip data to registry if needed
                // For now, no flip
            }
            
            cre_RendererCore_DrawSprite(
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
