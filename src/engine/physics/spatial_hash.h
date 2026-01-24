#ifndef SPATIAL_HASH_H
#define SPATIAL_HASH_H

#include <stdint.h> // for uint32_t etc.

typedef struct SpatialNode {
    struct SpatialNode* next; // next node
    uint32_t entityID; // matches e.id
    int16_t gridX;
    int16_t gridY;
} SpatialNode;

// Reset the hash at the start of every frame
void SpatialHash_Clear(void);

// Add an entity to the hash
void SpatialHash_Add(uint32_t entityID,int x,int y,int width,int height);

// query the hash
int SpatialHash_Query(int x,int y,int width,int height, uint32_t* results,int maxResults);
#endif