#include "cre_RenderSystem.h"
#include "cre_RendererCore.h"
#include "entity_manager.h"
#include "entity_registry.h"
#include "../physics/spatial_hash.h"
#include "config.h"
#include "atlas_data.h"

void cre_RenderSystem_DrawEntities(EntityRegistry* reg, Rectangle cullRect) {
    if (!reg) return;
    
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
            Vector2 position = {reg->pos_x[id], reg->pos_y[id]};
            Vector2 size = {reg->size_w[id], reg->size_h[id]};
            
            // Entity position is top-left of collision box
            // DrawSprite uses center pivot, so offset to center of collision box
            position.x += size.x * 0.5f;
            position.y += size.y * 0.5f;
            
            // Animation flip data - check if entity has animation component
            bool flipX = false;
            bool flipY = false;
            if (reg->component_masks[id] & COMP_ANIMATION) {
                // TODO: Add flip data to registry if needed
                // For now, no flip
            }
            
            cre_RendererCore_DrawSprite(
                reg->sprite_ids[id],
                position,
                size,
                (Vector2){0.5f, 0.5f}, // Central Pivot
                reg->rotation[id],
                flipX,
                flipY,
                reg->colors[id]
            );
        }
    }
}
