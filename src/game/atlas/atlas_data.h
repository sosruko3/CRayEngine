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
    { 4, 4, 192, 256 }, // character_zombie_run0
    { 4, 268, 192, 256 }, // character_zombie_run1
    { 4, 532, 192, 256 }, // character_zombie_run2
    { 4, 796, 124, 123 }, // player_idle0
    { 4, 927, 64, 64 }, // fish_blue
    { 4, 999, 24, 24 }, // enemy_idle
    { 4, 1031, 16, 16 }, // cactus
    { 4, 1055, 16, 16 }, // missing
};
#endif