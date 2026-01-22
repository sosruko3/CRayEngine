#include "viewport.h"
#include "raylib.h"
#include "config.h"

#define RESIZE_DEBOUNCE_FRAMES 12

static ViewportSize virtualView = {0};
static int resizeTimer = 0;

void Viewport_Init(int windowWidth, int windowHeight, float targetHeight) {
    virtualView.height = targetHeight;
    virtualView.aspect = (float)windowWidth / (float)windowHeight;
    virtualView.width = targetHeight * virtualView.aspect;
}
ViewportSize Viewport_Get(void) {
    return virtualView;

}

bool Viewport_ShouldResize(void) {
    if (IsWindowResized()) {
        resizeTimer = RESIZE_DEBOUNCE_FRAMES;
    }
    if (resizeTimer > 0) {
        resizeTimer--;

        if (resizeTimer == 0) {
            Viewport_Init(GetScreenWidth(),GetScreenHeight(),GAME_VIRTUAL_HEIGHT);
            return true;
        }
    }
}