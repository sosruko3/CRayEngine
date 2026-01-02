#include "game.h"
#include "entities/snake.h"
#include "entities/food.h"
#include "engine/core/config.h"
#include "engine/core/ui_helper.h"
#include "raylib.h"
#include "game_config.h"
#include "engine/core/game_types.h"
#include <stdio.h> // For sprintf

 // Global variables for game state
typedef struct {
    GameState state;
    int score;
} CoreState;

 static CoreState core;

 // --- Forward Declarations of Helper Functions ---
// These keep the main Update/Draw functions clean
static void UpdateMenu(void);
static void UpdatePlaying(void);
static void UpdateGameOver(void);

static void DrawMenu(void);
static void DrawPlaying(void);
static void DrawGameOver(void);

static void ResetGameplay(void);

void Game_Init(void)  {
    core.state = GAME_STATE_MENU;
    core.score = 0;
    ResetGameplay();
}

void Game_Update(void) {
    // Traffic cop. Delegates logic
    switch (core.state) {
        case GAME_STATE_MENU:     UpdateMenu(); break;
        case GAME_STATE_PLAYING:  UpdatePlaying(); break;
        case GAME_STATE_GAMEOVER: UpdateGameOver(); break;
    }
}
void Game_Draw(void) 
{
    ClearBackground(DARKGRAY);

     switch (core.state) {
        case GAME_STATE_MENU:       DrawMenu(); break;
        case GAME_STATE_PLAYING:    DrawPlaying(); break;
        case GAME_STATE_GAMEOVER:   DrawGameOver(); break;
    }
}
void Game_Shutdown(void) {
    // Cleanup if needed
}


   static void ResetGameplay(void)
{
    InitSnake(GRID_WIDTH/2,GRID_HEIGHT/2);
    InitFood();
    core.score = 0;
}

 static void UpdateMenu(void)
{
    if (IsKeyPressed(KEY_ENTER))
    {
        ResetGameplay();
        core.state = GAME_STATE_PLAYING;
    }
}

 static void DrawMenu(void)
{
    DrawTextCentered(MENU_TITLE_TEXT, SCREEN_HEIGHT/2-80,FONT_SIZE_TITLE,DARKGREEN);
    DrawTextCentered(MENU_START_TEXT,SCREEN_HEIGHT/2+20,FONT_SIZE_SUBTITLE,RAYWHITE);
}

 // For playing state
static void UpdatePlaying(void)
{
    UpdateSnake();
    UpdateFood();

    Vector2 head = GetSnakeHeadPosition();
    Vector2 foodPos = GetFoodPosition();

    // Check Food and Snake collision
    if ((int)head.x == (int)foodPos.x && (int)head.y == (int)foodPos.y) 
    {
        GrowSnake();
        SpawnFood(GRID_WIDTH,GRID_HEIGHT); // Remove magic numbers later on
        core.score += SCORE_PER_FOOD;
    }

     // CHeck Game over Collision
    if (CheckSnakeCollision(GRID_WIDTH,GRID_HEIGHT))
    {
        core.state = GAME_STATE_GAMEOVER;
    }
    
}

 static void DrawPlaying(void)
{
    DrawSnake();
    DrawFood();
    char scoreText[20];
    sprintf(scoreText,"Score: %d", core.score);
    DrawText(scoreText,SCREEN_WIDTH * 0.02f,SCREEN_HEIGHT * 0.02f,FONT_SIZE_SCORE,RAYWHITE);
}

 // For GAMEOVER

 static void UpdateGameOver(void)
{
    if (IsKeyPressed(KEY_ENTER))
    {
        core.state = GAME_STATE_MENU;
    }
}
static void DrawGameOver(void)
{
    DrawPlaying();
    DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT, (Color){0,0,0,200});

    DrawTextCentered(GAMEOVER_TITLE_TEXT,SCREEN_HEIGHT/2-50,FONT_SIZE_TITLE,RED);
    DrawTextCentered(GAMEOVER_RESTART_TEXT,SCREEN_HEIGHT/2+50,FONT_SIZE_SUBTITLE,RAYWHITE);
}