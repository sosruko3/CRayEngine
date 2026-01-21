#ifndef GENERATED_ATLAS_H
#define GENERATED_ATLAS_H
#include "raylib.h"

typedef enum {
    SPR_character_zombie_run0,
    SPR_character_zombie_run1,
    SPR_character_zombie_run2,
    SPR_player_idle0,
    SPR_fish_blue,
    SPR_enemy_idle,
    SPR_cactus,
    SPR_missing,
    SPR_COUNT
} SpriteID;

static const Rectangle atlas_rects[] = {
    { 2, 2, 192, 256 }, // character_zombie_run0
    { 2, 262, 192, 256 }, // character_zombie_run1
    { 2, 522, 192, 256 }, // character_zombie_run2
    { 2, 782, 124, 123 }, // player_idle0
    { 2, 909, 64, 64 }, // fish_blue
    { 2, 977, 24, 24 }, // enemy_idle
    { 2, 1005, 16, 16 }, // cactus
    { 2, 1025, 16, 16 }, // missing
};
#endif