#ifndef SPATIAL_HASH_H
#define SPATIAL_HASH_H

#include <stdint.h>

/**
 * @brief Index-linked spatial node for dual-layer spatial hash.
 * 
 * Uses index-based linking (uint32_t nextIdx) instead of raw pointers
 * for cache-friendliness and Data-Oriented Design. Packed to 16 bytes.
 */
typedef struct SpatialNode {
    uint32_t entityID;   ///< Entity identifier
    uint32_t nextIdx;    ///< Index into node pool (UINT32_MAX = null sentinel)
    int16_t  gridX;      ///< Grid cell X coordinate
    int16_t  gridY;      ///< Grid cell Y coordinate
    uint32_t _padding;   ///< Padding to 16 bytes
} SpatialNode;

// ============================================================================
// Dynamic Layer API (cleared every frame)
// ============================================================================

/**
 * @brief Clears dynamic buckets and resets dynamic pool. Call once per frame.
 */
void SpatialHash_ClearDynamic(void);

/**
 * @brief Adds a dynamic entity to the spatial hash (inv_mass > 0).
 * 
 * @param entityID Entity identifier
 * @param x World X position
 * @param y World Y position
 * @param width Entity width
 * @param height Entity height
 */
void SpatialHash_AddDynamic(uint32_t entityID, int x, int y, int width, int height);

// ============================================================================
// Static Layer API (persistent until removed)
// ============================================================================

/**
 * @brief Adds a static entity to the spatial hash (inv_mass <= 0).
 * 
 * Static entities persist until explicitly removed.
 * 
 * @param entityID Entity identifier
 * @param x World X position
 * @param y World Y position
 * @param width Entity width
 * @param height Entity height
 */
void SpatialHash_AddStatic(uint32_t entityID, int x, int y, int width, int height);

/**
 * @brief Removes a static entity from the spatial hash.
 * 
 * Unlinks from all buckets the entity occupies and recycles nodes.
 * 
 * @param entityID Entity identifier
 * @param x World X position
 * @param y World Y position
 * @param width Entity width
 * @param height Entity height
 */
void SpatialHash_RemoveStatic(uint32_t entityID, int x, int y, int width, int height);

// ============================================================================
// Full Clear API (scene transitions)
// ============================================================================

/**
 * @brief Clears both static and dynamic layers. Use during scene transitions.
 */
void SpatialHash_ClearAll(void);

// ============================================================================
// Query API
// ============================================================================

/**
 * @brief Queries both static and dynamic layers for entities in the given area.
 * 
 * Results are deduplicated via linear scan.
 * 
 * @param x World X position
 * @param y World Y position
 * @param width Query area width
 * @param height Query area height
 * @param results Output array for entity IDs
 * @param maxResults Maximum number of results to return
 * @return Number of unique entities found
 */
int SpatialHash_Query(int x, int y, int width, int height, uint32_t* results, int maxResults);

#endif