#include "entity_manager.h"
// entity.h is included too
#include <stdio.h>
#include "logger.h"
#include <string.h>


EntityData entityStore[MAX_ENTITIES];

// Free list implemention
static uint32_t freeList[MAX_ENTITIES];
static uint32_t freeCount;

static uint32_t activeCount;


void EntityManager_Init(void) {
    EntityManager_Reset();
}
void EntityManager_Reset(void) {
    // Clean the memory
    memset(entityStore,0,sizeof(entityStore));

    activeCount = 0;
    freeCount = MAX_ENTITIES;

    // fill the box
    for (uint32_t i =0 ;i< MAX_ENTITIES;i++) {
        freeList[i] = (MAX_ENTITIES -1) -i;
    }
    Log(LOG_LVL_INFO,"Entity Manager Reset Complete");
}

Entity EntityManager_Create(int type,Vector2 pos) {
    if (freeCount == 0) {
        Log(LOG_LVL_ERROR,"Entity Manager is full!");
        return (Entity){ .id = 0xFFFFFFFF };
    }

    // Pop a key from the box
    uint32_t index = freeList[--freeCount];

    // initialize the slot
    entityStore[index ] = (EntityData) {
        .isActive = true,
        .type = type,
        .position = pos,
        .velocity = {0,0},
        .rotation = 0.0f,
        .scale = {1.0f,1.0f},
        .radius = 16.0f,
        .color = RED
    };

    activeCount++;

    // return the ticket
    return (Entity){ .id = index};
}

void EntityManager_Destroy(Entity e) {

    EntityData* data = EntityManager_Get(e);
    if (data == NULL) return;

    data->isActive = false;

    freeList[freeCount++] = e.id;

    activeCount--;
}

void EntityManager_Update(float dt) {
    for (uint32_t i = 0;i < MAX_ENTITIES;i++) {
        if ( entityStore[i].isActive) {
            entityStore[i].position.x += entityStore[i].velocity.x * dt;
            entityStore[i].position.y += entityStore[i].velocity.y * dt;
        }
    }
}

void EntityManager_Shutdown(void) {
    memset(entityStore,0,sizeof(entityStore));
    // memset do not free pointers do not forget that
    freeCount = 0;
    activeCount = 0;
}