#ifndef GENERATED_ATLAS_H
#define GENERATED_ATLAS_H
#include "raylib.h"

// 1. The Symbolic IDs
typedef enum {
    SPR_character_zombie_run0,
    SPR_character_zombie_run1,
    SPR_character_zombie_run2,
    SPR_player_idle0,
    SPR_fish_blue,
    SPR_Untitled0,
    SPR_Untitled1,
    SPR_soldier,
    SPR_enemy_idle,
    SPR_cactus,
    SPR_missing,
    SPR_COUNT
} SpriteID;

// 2. The Data
static const Rectangle atlas_rects[] = {
    { 2, 2, 192, 256 }, // character_zombie_run0
    { 2, 262, 192, 256 }, // character_zombie_run1
    { 2, 522, 192, 256 }, // character_zombie_run2
    { 2, 782, 124, 123 }, // player_idle0
    { 2, 909, 64, 64 }, // fish_blue
    { 2, 977, 32, 32 }, // Untitled0
    { 2, 1013, 32, 32 }, // Untitled1
    { 2, 1049, 32, 32 }, // soldier
    { 2, 1085, 24, 24 }, // enemy_idle
    { 2, 1113, 16, 16 }, // cactus
    { 2, 1133, 16, 16 }, // missing
};

// 3. Automated Helper Macro
// This allows you to define an animation by just providing the prefix and frame count
#define GET_FRAME(prefix, frame) SPR_##prefix##frame

#endif