#include "engine.h"
#include "raylib.h"
#include <stdio.h>

void Engine_Init(int width, int height, const char* title) {
    InitWindow(width, height, title);
    SetTargetFPS(60);
    printf("Engine Initialized: %dx%d - %s\n", width, height, title);
}

void Engine_Run(GameUpdateCallback update, GameDrawCallback draw) {
    while (!WindowShouldClose()) {
        // Update
        if (update) {
            update();
        }

        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        if (draw) {
            draw();
        }

        EndDrawing();
    }
}

void Engine_Shutdown(void) {
    CloseWindow();
    printf("Engine Shutdown\n");
}
