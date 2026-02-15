    #include "cre_spatialHash.h"
    #include <stddef.h>
    #include <stdbool.h>
    #include <string.h>
    #include "engine/core/cre_logger.h"
    #include "engine/core/cre_config.h"
    #include <assert.h>

    // ============================================================================
    // Configuration Constants
    // ============================================================================

    #define SPATIAL_MAX_STATIC  40000
    #define SPATIAL_MAX_DYNAMIC 20000
    #define SPATIAL_NULL_IDX    UINT32_MAX

    // #4 Optimization: Power-of-2 bitmask for fast hashing (replaces modulo)
    #define SPATIAL_HASH_MASK   (SPATIAL_HASH_SIZE - 1)

    // Faster division, Bit-shifting.
    #define WORLD_TO_GRID(val) ((val) >> SPATIAL_GRID_SHIFT)

    // Compile-time validation
    static_assert((1 << SPATIAL_GRID_SHIFT) == SPATIAL_GRID_SIZE, 
                "SPATIAL_GRID_SHIFT must match log2 of SPATIAL_GRID_SIZE!");
    static_assert((SPATIAL_HASH_SIZE & SPATIAL_HASH_MASK) == 0,
                "SPATIAL_HASH_SIZE must be power of 2 for bitmask optimization!");

    // ============================================================================
    // #1 Optimization: Timestamp-based O(1) Deduplication
    // ============================================================================
    // Instead of O(N) linear scan, use per-entity timestamp tracking.
    // Entity is duplicate if lastSeenFrame[id] == currentQueryFrame.

    static uint32_t lastSeenFrame[MAX_ENTITIES];  // 64KB for 16384 entities
    static uint32_t currentQueryFrame = 0;

    /**
     * @brief O(1) duplicate check using timestamp comparison.
     * @return true if already seen this query, false if first time (and marks as seen)
     */
    static inline bool MarkSeen(uint32_t entityID) {
        if (lastSeenFrame[entityID] == currentQueryFrame) {
            return true;  // Duplicate
        }
        lastSeenFrame[entityID] = currentQueryFrame;
        return false;  // First time
    }

    /**
     * @brief Handle timestamp wraparound (every ~4 billion queries).
     */
    static inline void AdvanceQueryFrame(void) {
        currentQueryFrame++;
        if (currentQueryFrame == 0) {
            // Overflow - reset everything to avoid false positives
            memset(lastSeenFrame, 0, sizeof(lastSeenFrame));
            currentQueryFrame = 1;
        }
    }

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
    // Chipmunk2D's Hashing Algorithm (Optimized with bitmask)
    // ============================================================================

    static inline int GetHashIndex(int cellX, int cellY) {
        unsigned int h1 = (unsigned int)cellX * 73856093;
        unsigned int h2 = (unsigned int)cellY * 19349663;
        return (int)((h1 ^ h2) & SPATIAL_HASH_MASK);  // Bitmask, not modulo
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
        const int minX = WORLD_TO_GRID(x);
        const int minY = WORLD_TO_GRID(y);
        const int maxX = WORLD_TO_GRID(x + width);
        const int maxY = WORLD_TO_GRID(y + height);

        for (int cy = minY; cy <= maxY; cy++) {
            // #4 Optimization: Precompute y-component of hash once per row
            const unsigned int hy = (unsigned int)cy * 19349663;
            
            for (int cx = minX; cx <= maxX; cx++) {
                // Inline hash calculation (avoids function call overhead)
                const unsigned int hx = (unsigned int)cx * 73856093;
                const int bucketIndex = (int)((hx ^ hy) & SPATIAL_HASH_MASK);

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
    // Merged Query (with O(1) Timestamp Deduplication)
    // ============================================================================

    int SpatialHash_Query(int x, int y, int width, int height, uint32_t* results, int maxResults) {
        // #1 Optimization: Advance query frame for O(1) deduplication
        AdvanceQueryFrame();
        
        int count = 0;

        const int minX = WORLD_TO_GRID(x);
        const int minY = WORLD_TO_GRID(y);
        const int maxX = WORLD_TO_GRID(x + width);
        const int maxY = WORLD_TO_GRID(y + height);

        for (int cy = minY; cy <= maxY; cy++) {
            for (int cx = minX; cx <= maxX; cx++) {
                const int bucketIndex = GetHashIndex(cx, cy);

                // Query static layer
                uint32_t curr = staticBuckets[bucketIndex];
                while (curr != SPATIAL_NULL_IDX) {
                    SpatialNode* node = &staticNodes[curr];
                    if (node->gridX == cx && node->gridY == cy) {
                        // #1 Optimization: O(1) duplicate check via timestamp
                        if (!MarkSeen(node->entityID)) {
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
                        // #1 Optimization: O(1) duplicate check via timestamp
                        if (!MarkSeen(node->entityID)) {
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