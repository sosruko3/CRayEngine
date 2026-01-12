#include "spatial_hash.h"
#include <stdlib.h> //for abs()
#include <stddef.h> //for NULL
#include "../core/logger.h"

// Actual buckets (Arrays of pointers)
static SpatialNode* buckets[SPATIAL_HASH_SIZE];
// Node pool
static SpatialNode nodePool[SPATIAL_MAX_NODES];
static int nodePoolIndex = 0;

// Chipmunk2D's HASHING ALGORITHM
// Decided using this algorithm, it is great.

// Maps(GridX,GridY) -> Bucket Index
static int GetHashIndex(int cellX,int cellY) {
    // These are the prime numbers used in spatial hashing to prevent patterns.
    // We cast to unsigned int to handle negative coordinates correctly.
    unsigned int h1 = (unsigned int)cellX * 73856093;
    unsigned int h2 = (unsigned int)cellY * 19349663;

    // XOR (^) mixes the X and Y values.
    // Modulo (%) maps it to our array size;
    unsigned int n = (h1 ^ h2) % SPATIAL_HASH_SIZE;
    return (int)n;
}
static SpatialNode* PopNode() {
    if (nodePoolIndex >= SPATIAL_MAX_NODES) {
        Log(LOG_LVL_WARNING,"Node pool is FULL.");
        // returning null to prevent a crash.
        return NULL;
    }
    return &nodePool[nodePoolIndex++];
}
// api implementation
void SpatialHash_Clear(void) {
    // clear the bucket heads
    for (int i = 0;i < SPATIAL_HASH_SIZE;i++) {
        buckets[i] = NULL;
    }
    nodePoolIndex = 0;
}
void SpatialHash_Add(uint32_t entityID,int x,int y,int width,int height) {
    // Calculate grid coordinates
    // floor the values to handle negative coordinates properly if needed
    int minX = x/SPATIAL_GRID_SIZE;
    int minY = y/SPATIAL_GRID_SIZE;
    int maxX = (x + width) / SPATIAL_GRID_SIZE;
    int maxY = (y + height) / SPATIAL_GRID_SIZE;

    // loop through every cell this entity touches (up to 4 cells with corners)
    for (int cy = minY;cy <= maxY;cy++) {
        for (int cx = minX;cx <= maxX;cx++) {
            // calculate the hash first
            int bucketIndex = GetHashIndex(cx, cy);

            // allocate a node
            SpatialNode* newNode = PopNode();
            if (newNode == NULL) return; // pool full

            // store data
            newNode->entityID = entityID;

            // link into list
            newNode->next = buckets[bucketIndex];
            buckets[bucketIndex] = newNode;
        }
    }
}
int SpatialHash_Query(int x,int y,int width,int height, uint32_t* results,int maxResults) {
    int count = 0;

    int minX = x / SPATIAL_GRID_SIZE;
    int minY = y / SPATIAL_GRID_SIZE;
    int maxX = (x + width) / SPATIAL_GRID_SIZE;
    int maxY = (y + height) / SPATIAL_GRID_SIZE;

    for (int cy= minY;cy <= maxY;cy++) {
        for (int cx = minX;cx <= maxX;cx++) {
            int bucketIndex = GetHashIndex(cx,cy);
            SpatialNode* current = buckets[bucketIndex];

            while (current != NULL) {
                if (count < maxResults) {
                    results[count++] = current->entityID;
                }
                else {
                    return count; // result buffer is full
                }
                current = current->next;
            }
        }
    }
    return count;
}

















