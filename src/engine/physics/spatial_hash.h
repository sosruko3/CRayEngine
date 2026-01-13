#ifndef SPATIAL_HASH_H
#define SPATIAL_HASH_H

#include <stdint.h> // for uint32_t
// Spatial hash of this engine will be using algorithm from Chipmunk2D

// 128 pixel is a standart bucket size
#define SPATIAL_GRID_SIZE 128

// Size of the hash table (number of buckets)
// 1009 is a prime number, which helps spread the data evenly. NOTE: Changed 1009 to 4096 for optimization.
#define SPATIAL_HASH_SIZE 4096

// Total memory pool for nodes.
// Number is big in case entites touch multiple of them. In theory this amount can withstand 10k entity.
#define SPATIAL_MAX_NODES 40000

typedef struct SpatialNode {
    uint32_t entityID; // matches e.id
    struct SpatialNode* next; // next node
} SpatialNode;

// Reset the hash at the start of every frame
void SpatialHash_Clear(void);

// Add an entity to the hash
void SpatialHash_Add(uint32_t entityID,int x,int y,int width,int height);

// query the hash
int SpatialHash_Query(int x,int y,int width,int height, uint32_t* results,int maxResults);
#endif