#ifndef ENTITY_TYPES_H
#define ENTITY_TYPES_H

typedef enum {
    TYPE_PLAYER = 0,
    TYPE_ENEMY,
    TYPE_BULLET,
    TYPE_FOOD,
    TYPE_WALL,
    TYPE_PARTICLE
} EntityType;

#define L_PLAYER   (1 << TYPE_PLAYER)    // 1
#define L_ENEMY    (1 << TYPE_ENEMY)     // 2
#define L_BULLET   (1 << TYPE_BULLET)    // 4
#define L_FOOD     (1 << TYPE_FOOD)      // 8
#define L_WALL     (1 << TYPE_WALL)      // 16
#define L_PARTICLE (1 << TYPE_PARTICLE)  // 32

#endif