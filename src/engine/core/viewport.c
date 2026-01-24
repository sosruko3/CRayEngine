#include "viewport.h"
#include "raylib.h"
#include "config.h"
#include "logger.h"

#define RESIZE_DEBOUNCE_FRAMES 12

static ViewportSize virtualView = {0};
static int resizeTimer = 0;
static bool s_didResizeThisFrame = false;

static void CalculateInternal(int w,int h) {
    virtualView.height = GAME_VIRTUAL_HEIGHT;
    virtualView.aspect = (float)w / (float)h;
    virtualView.width = (float)GAME_VIRTUAL_HEIGHT * virtualView.aspect;
}
void Viewport_Init(int initalW,int initialH) {
    CalculateInternal(initalW,initialH);
}
ViewportSize Viewport_Get(void) {
    return virtualView;
}
void Viewport_Update(void) {
    s_didResizeThisFrame = false;
    if (IsWindowResized()) {
        resizeTimer = RESIZE_DEBOUNCE_FRAMES;
    }
    if (resizeTimer > 0) {
        resizeTimer--;

        if (resizeTimer == 0) {
            CalculateInternal(GetScreenWidth(),GetScreenHeight());
            s_didResizeThisFrame = true;
            Log(LOG_LVL_DEBUG, "Viewport Resized [Debounced]");
        }
    }
}
bool Viewport_ShouldResize(void) {
    return s_didResizeThisFrame;
}