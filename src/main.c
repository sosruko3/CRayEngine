#include "engine/core/engine.h"
#include "engine/core/config.h"
#include "game/game_config.h"
#include "game/game.h"

int main(void) {
    Engine_Init(SCREEN_WIDTH,SCREEN_HEIGHT,GAME_TITLE);
    
    Game_Init(); // Initialize game state

    // Pass the functions from game.c to engine
    Engine_Run(Game_Update, Game_Draw);
    
    Game_Shutdown(); // Cleanup game state
    
    Engine_Shutdown();
    return 0;
}
