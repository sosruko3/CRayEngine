#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include <stdint.h>
#define SETTING_CFG_PATH "src/game/config/settings.cfg"
#define GAME_TITLE "CRayEngine"
#define MENU_TITLE_TEXT "Snake Game"
#define MENU_START_TEXT "Press ENTER to START"
#define MENU_TO_QUIT "Press ESC to Quit"
#define GAMEOVER_TITLE_TEXT "Game Over"
#define GAMEOVER_RESTART_TEXT "Press ENTER for Menu"

#define FONT_SIZE_TITLE 80
#define FONT_SIZE_SUBTITLE 40
#define FONT_SIZE_SCORE 80

#define dirCONFIG "assets/config/settings.cfg"

constexpr uint8_t RENDER_LAYER_DEFAULT = 0u;
constexpr uint8_t RENDER_LAYER_ENEMY = 10u;
constexpr uint8_t RENDER_LAYER_PLAYER = 20u;

#endif
