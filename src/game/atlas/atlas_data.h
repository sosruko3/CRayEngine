#ifndef GENERATED_ATLAS_H
#define GENERATED_ATLAS_H
#include "raylib.h"

typedef enum {
    SPR_enemy_idle,
    SPR_player_idle0,
    SPR_player_idle1,
    SPR_player_idle2,
    SPR_player_idle3,
    SPR_player_idle4,
    SPR_player_idle5,
    SPR_player_idle6,
    SPR_player_idle7,
    SPR_cactus,
    SPR_missing,
    SPR_COUNT
} SpriteID;

static const Rectangle atlas_rects[] = {
    { 2, 2, 24, 24 }, // enemy_idle
    { 2, 30, 24, 24 }, // player_idle0
    { 2, 58, 24, 24 }, // player_idle1
    { 2, 86, 24, 24 }, // player_idle2
    { 2, 114, 24, 24 }, // player_idle3
    { 2, 142, 24, 24 }, // player_idle4
    { 2, 170, 24, 24 }, // player_idle5
    { 2, 198, 24, 24 }, // player_idle6
    { 2, 226, 24, 24 }, // player_idle7
    { 2, 254, 16, 16 }, // cactus
    { 2, 274, 16, 16 }, // missing
};
#endif