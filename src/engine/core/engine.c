#include "engine.h"
#include "raylib.h"
#include <stdio.h>
#include "config.h"
#include "logger.h"
#include <stdlib.h>
#include "scene_manager.h"
#include "scene.h"


void Engine_Init(int width, int height, const char* title) {
    Logger_Init();
    Log(LOG_LVL_INFO,"Engine Initializing...");
    Log(LOG_LVL_DEBUG,"Target Resolution: %dx%d",width,height);

    InitWindow(width, height, title);
    SetTargetFPS(TARGET_FRAMERATE);

    if (!IsWindowReady()) {
        Log(LOG_LVL_ERROR,"CRITICAL: Raylib failed to create window. ");

        Logger_Shutdown();
        exit(1);
    }

    SceneManager_Init();

    Log(LOG_LVL_INFO,"Windows created successfully.");
}

void Engine_Run() {
    Log(LOG_LVL_INFO,"Entering main loop");
    while (!WindowShouldClose()) {
        // Update
        SceneManager_Update();

        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        SceneManager_Draw();

        EndDrawing();
    }
    Log(LOG_LVL_INFO,"Main loop exited. (Window closed)");
}

void Engine_Shutdown(void) {
    Log(LOG_LVL_INFO,"Shutting down Raylib...");
    CloseWindow();
    SceneManager_Shutdown();

    // Add failsafes later on.
    Log(LOG_LVL_INFO,"Engine Shutdown Complete.");
    Logger_Shutdown();
}
