#include "spatial_hash.h"
#include <stddef.h>
#include <stdbool.h>
#include "engine/core/logger.h"
#include "engine/core/config.h"
#include <assert.h>

// ============================================================================
// Configuration Constants
// ============================================================================

#define SPATIAL_MAX_STATIC  40000
#define SPATIAL_MAX_DYNAMIC 20000
#define SPATIAL_NULL_IDX    UINT32_MAX

#define WORLD_TO_GRID(val) ((val) >> SPATIAL_GRID_SHIFT)

// Compile-time validation
static_assert((1 << SPATIAL_GRID_SHIFT) == SPATIAL_GRID_SIZE, 
              "SPATIAL_GRID_SHIFT must match log2 of SPATIAL_GRID_SIZE!");

// ============================================================================
// Dual-Layer Data Structures
// ============================================================================

// Bucket head indices (SPATIAL_NULL_IDX = empty)
static uint32_t staticBuckets[SPATIAL_HASH_SIZE];
static uint32_t dynamicBuckets[SPATIAL_HASH_SIZE];

// Separate node pools for each layer
static SpatialNode staticNodes[SPATIAL_MAX_STATIC];
static SpatialNode dynamicNodes[SPATIAL_MAX_DYNAMIC];

// Allocation counters
static uint32_t staticPoolIdx  = 0;
static uint32_t dynamicPoolIdx = 0;

// Static layer free-list (for node reuse on removal)
static uint32_t staticFreeHead = SPATIAL_NULL_IDX;

// ============================================================================
// Chipmunk2D's Hashing Algorithm
// ============================================================================

static int GetHashIndex(int cellX, int cellY) {
    unsigned int h1 = (unsigned int)cellX * 73856093;
    unsigned int h2 = (unsigned int)cellY * 19349663;
    unsigned int n = (h1 ^ h2) % SPATIAL_HASH_SIZE;
    return (int)n;
}

// ============================================================================
// Static Layer Node Allocation (with Free-List)
// ============================================================================

/**
 * @brief Pop a static node from free-list or allocate from pool.
 * @return Node index, or SPATIAL_NULL_IDX if pool exhausted.
 */
static uint32_t PopStaticNode(void) {
    // First try free-list
    if (staticFreeHead != SPATIAL_NULL_IDX) {
        uint32_t idx = staticFreeHead;
        staticFreeHead = staticNodes[idx].nextIdx;
        return idx;
    }
    // Otherwise allocate from pool
    if (staticPoolIdx >= SPATIAL_MAX_STATIC) {
        Log(LOG_LVL_WARNING, "Static node pool is FULL.");
        return SPATIAL_NULL_IDX;
    }
    return staticPoolIdx++;
}

/**
 * @brief Push a static node index onto the free-list for reuse.
 */
static void PushStaticFree(uint32_t idx) {
    staticNodes[idx].nextIdx = staticFreeHead;
    staticFreeHead = idx;
}

// ============================================================================
// Dynamic Layer Node Allocation
// ============================================================================

/**
 * @brief Allocate a dynamic node from pool.
 * @return Node index, or SPATIAL_NULL_IDX if pool exhausted.
 */
static uint32_t PopDynamicNode(void) {
    if (dynamicPoolIdx >= SPATIAL_MAX_DYNAMIC) {
        // Silently return on overflow as per plan
        return SPATIAL_NULL_IDX;
    }
    return dynamicPoolIdx++;
}

// ============================================================================
// Clear Functions
// ============================================================================

void SpatialHash_ClearDynamic(void) {
    // 0xFF is -1 in 2's complement, which matches UINT32_MAX
    memset(dynamicBuckets,0xFF,sizeof(dynamicBuckets));
    dynamicPoolIdx = 0;
}

void SpatialHash_ClearAll(void) {
    // Clear Buckets (Fast SIMD wipe)
    // 0xFF sets all bits to 1, creating UINT32_MAX
    memset(staticBuckets,  0xFF, sizeof(staticBuckets));
    memset(dynamicBuckets, 0xFF, sizeof(dynamicBuckets));
    
    // Reset pools and free-list
    staticPoolIdx  = 0;
    dynamicPoolIdx = 0;
    staticFreeHead = SPATIAL_NULL_IDX;
}

// ============================================================================
// Add Functions
// ============================================================================

void SpatialHash_AddStatic(uint32_t entityID, int x, int y, int width, int height) {
    int minX = WORLD_TO_GRID(x);
    int minY = WORLD_TO_GRID(y);
    int maxX = WORLD_TO_GRID(x + width);
    int maxY = WORLD_TO_GRID(y + height);

    for (int cy = minY; cy <= maxY; cy++) {
        for (int cx = minX; cx <= maxX; cx++) {
            int bucketIndex = GetHashIndex(cx, cy);

            uint32_t nodeIdx = PopStaticNode();
            if (nodeIdx == SPATIAL_NULL_IDX) return;

            staticNodes[nodeIdx].entityID = entityID;
            staticNodes[nodeIdx].gridX    = (int16_t)cx;
            staticNodes[nodeIdx].gridY    = (int16_t)cy;
            staticNodes[nodeIdx].nextIdx  = staticBuckets[bucketIndex];
            staticBuckets[bucketIndex]    = nodeIdx;
        }
    }
}

void SpatialHash_AddDynamic(uint32_t entityID, int x, int y, int width, int height) {
    int minX = WORLD_TO_GRID(x);
    int minY = WORLD_TO_GRID(y);
    int maxX = WORLD_TO_GRID(x + width);
    int maxY = WORLD_TO_GRID(y + height);

    for (int cy = minY; cy <= maxY; cy++) {
        for (int cx = minX; cx <= maxX; cx++) {
            int bucketIndex = GetHashIndex(cx, cy);

            uint32_t nodeIdx = PopDynamicNode();
            if (nodeIdx == SPATIAL_NULL_IDX) return;

            dynamicNodes[nodeIdx].entityID = entityID;
            dynamicNodes[nodeIdx].gridX    = (int16_t)cx;
            dynamicNodes[nodeIdx].gridY    = (int16_t)cy;
            dynamicNodes[nodeIdx].nextIdx  = dynamicBuckets[bucketIndex];
            dynamicBuckets[bucketIndex]    = nodeIdx;
        }
    }
}

// ============================================================================
// Remove Static (with prev-trick for unlinking)
// ============================================================================

void SpatialHash_RemoveStatic(uint32_t entityID, int x, int y, int width, int height) {
    int minX = WORLD_TO_GRID(x);
    int minY = WORLD_TO_GRID(y);
    int maxX = WORLD_TO_GRID(x + width);
    int maxY = WORLD_TO_GRID(y + height);

    for (int cy = minY; cy <= maxY; cy++) {
        for (int cx = minX; cx <= maxX; cx++) {
            int bucketIndex = GetHashIndex(cx, cy);

            uint32_t prev = SPATIAL_NULL_IDX;
            uint32_t curr = staticBuckets[bucketIndex];

            while (curr != SPATIAL_NULL_IDX) {
                SpatialNode* node = &staticNodes[curr];
                uint32_t next = node->nextIdx;

                // Match by entityID and cell coordinates
                if (node->entityID == entityID && 
                    node->gridX == cx && 
                    node->gridY == cy) {
                    // Unlink from list
                    if (prev == SPATIAL_NULL_IDX) {
                        staticBuckets[bucketIndex] = next;
                    } else {
                        staticNodes[prev].nextIdx = next;
                    }
                    // Recycle node
                    PushStaticFree(curr);
                    // Continue search (entity might be in bucket once per cell)
                    curr = next;
                    continue;
                }

                prev = curr;
                curr = next;
            }
        }
    }
}

// ============================================================================
// Merged Query (with Deduplication)
// ============================================================================

/**
 * @brief Check if entityID already exists in results array.
 */
static bool IsDuplicate(const uint32_t* results, int count, uint32_t entityID) {
    for (int i = 0; i < count; i++) {
        if (results[i] == entityID) return true;
    }
    return false;
}

int SpatialHash_Query(int x, int y, int width, int height, uint32_t* results, int maxResults) {
    int count = 0;

    int minX = WORLD_TO_GRID(x);
    int minY = WORLD_TO_GRID(y);
    int maxX = WORLD_TO_GRID(x + width);
    int maxY = WORLD_TO_GRID(y + height);

    for (int cy = minY; cy <= maxY; cy++) {
        for (int cx = minX; cx <= maxX; cx++) {
            int bucketIndex = GetHashIndex(cx, cy);

            // Query static layer
            uint32_t curr = staticBuckets[bucketIndex];
            while (curr != SPATIAL_NULL_IDX) {
                SpatialNode* node = &staticNodes[curr];
                if (node->gridX == cx && node->gridY == cy) {
                    if (!IsDuplicate(results, count, node->entityID)) {
                        if (count >= maxResults) return count;
                        results[count++] = node->entityID;
                    }
                }
                curr = node->nextIdx;
            }

            // Query dynamic layer
            curr = dynamicBuckets[bucketIndex];
            while (curr != SPATIAL_NULL_IDX) {
                SpatialNode* node = &dynamicNodes[curr];
                if (node->gridX == cx && node->gridY == cy) {
                    if (!IsDuplicate(results, count, node->entityID)) {
                        if (count >= maxResults) return count;
                        results[count++] = node->entityID;
                    }
                }
                curr = node->nextIdx;
            }
        }
    }

    return count;
}