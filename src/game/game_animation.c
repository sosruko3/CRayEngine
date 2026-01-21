#include "game_animation.h"
#include "atlas_data.h"

const AnimDef ANIMATIONS[ANIM_COUNT] = {
    //SpriteID           First frame     Frames perSec repeat 
    [ANIM_PLAYER_IDLE] = {SPR_player_idle0 ,1 ,1.0f ,false },
    [ANIM_ENEMY_IDLE]  = {SPR_player_idle0 ,3 ,6.0f ,true},
    [ANIM_CACTUS_IDLE] = {SPR_cactus       ,1,1.0f ,false},
    [ANIM_ZOMBIE_RUN] = {SPR_character_zombie_run0,3,6.0f,true},
};