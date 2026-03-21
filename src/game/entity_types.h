#ifndef ENTITY_TYPES_H
#define ENTITY_TYPES_H

#include <stdint.h>
enum EntityType : uint8_t {
  TYPE_PLAYER = 0,
  TYPE_ENEMY,
  TYPE_BULLET,
  TYPE_FOOD,
  TYPE_WALL,
  TYPE_PARTICLE,
  TYPE_CAMERA
};

constexpr uint8_t L_PLAYER = (1 << TYPE_PLAYER);
constexpr uint8_t L_ENEMY = (1 << TYPE_ENEMY);
constexpr uint8_t L_BULLET = (1 << TYPE_BULLET);
constexpr uint8_t L_FOOD = (1 << TYPE_FOOD);
constexpr uint8_t L_WALL = (1 << TYPE_WALL);
constexpr uint8_t L_PARTICLE = (1 << TYPE_PARTICLE);

#endif
