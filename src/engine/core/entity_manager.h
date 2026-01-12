#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include "entity.h"
#include <stddef.h>

#define MAX_ENTITIES 10000

extern EntityData entityStore[MAX_ENTITIES];

void EntityManager_Init(void);
void EntityManager_Reset(void);
Entity EntityManager_Create(int type,Vector2 pos);
void EntityManager_Destroy(Entity e);
void EntityManager_Shutdown(void);

static inline EntityData* EntityManager_Get(Entity e) {
    if (e.id >= MAX_ENTITIES) return NULL;
    if (!(entityStore[e.id].flags & FLAG_ACTIVE)) return NULL;
    if (entityStore[e.id].generation != e.generation) return NULL;
    // Direct memory access
    return &entityStore[e.id];
}
#endif