#include "entity_manager.h"
// entity.h is included too
#include <stdio.h>
#include "logger.h"
#include <string.h>
#include "atlas_data.h"
#include "config.h"
#include "animation.h"

EntityData entityStore[MAX_ENTITIES];

// Free List implemention
static uint32_t freeList[MAX_ENTITIES];
static uint32_t freeCount;
static uint32_t activeCount;

void EntityManager_Init(void) {
    EntityManager_Reset();
}
void EntityManager_Reset(void) {
    // Clean the memory , change this to something else later.
    memset(entityStore,0,sizeof(entityStore));
    activeCount = 0;
    freeCount = MAX_ENTITIES;

    for(int i = 0; i < MAX_ENTITIES; i++) {
        comp_anim[i] = (AnimComponent){0};
    }
    // fill the freelist
    for (uint32_t i =0 ;i< MAX_ENTITIES;i++) {
        freeList[i] = (MAX_ENTITIES -1)-i;
    }
    Log(LOG_LVL_INFO,"Entity Manager Reset Complete");
}

Entity EntityManager_Create(int type,Vector2 pos) {
    if (!freeCount) {
        //Log(LOG_LVL_ERROR,"Entity Manager is full!"); This prints 1000 log after Z button presssed(func for spawning 100 entity)
        return (Entity){ .id = 0xFFFFFFFF };
    }

    // Take a recycled index from the free list
    uint32_t index = freeList[--freeCount];
    uint32_t currentGen = entityStore[index].generation;

    // Initialize the data at that specific index
    entityStore[index] = (EntityData) {
        .flags = FLAG_ACTIVE | FLAG_VISIBLE | FLAG_SOLID,
        .generation = currentGen,
        .type = type,
        .position = pos,
        .velocity = {0,0},
        .rotation = 0.0f,
        .size = {32.0f,32.0f},
        .spriteID = SPR_missing,
        .color = WHITE
    };

    comp_anim[index] = (AnimComponent) {
        .timer = 0.0f,
        .currentAnimID = 0, // Usually your IDLE animation
        .finished = 0,
        .flipX = 0
    };
    activeCount++;

    // Hand the user a ticket with that number
    return (Entity){ .id = index, .generation = currentGen};
}

void EntityManager_Destroy(Entity e) {

    EntityData* data = EntityManager_Get(e);
    if (data == NULL) return;
    data->flags &= ~FLAG_ACTIVE;
    data->generation++;

    freeList[freeCount++] = e.id;
    activeCount--;
}
void EntityManager_Shutdown(void) {
    memset(entityStore,0,sizeof(entityStore));
    // memset do not free pointers do not forget that
    freeCount = 0;
    activeCount = 0;
}