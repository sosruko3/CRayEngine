#include "cre_viewport.h"
#include "engine/core/cre_config.h"
#include "engine/core/cre_logger.h"
#include "raylib.h"
#include <stdint.h>

constexpr uint32_t RESIZE_DEBOUNCE_FRAMES = 12;

static ViewportSize virtualView = {};
static int resizeTimer = 0;
static bool s_didResizeThisFrame = false;
static int s_lastLoggedW = 0;
static int s_lastLoggedH = 0;

static void CalculateInternal(float w, float h) {
  virtualView.height = GAME_VIRTUAL_HEIGHT;
  virtualView.aspect = (w / h);
  virtualView.width = GAME_VIRTUAL_HEIGHT * virtualView.aspect;
}
void Viewport_Init(float initalW, float initialH) {
  CalculateInternal(initalW, initialH);
}
ViewportSize Viewport_Get(void) { return virtualView; }
void Viewport_Update(void) {
  s_didResizeThisFrame = false;
  if (IsWindowResized()) {
    resizeTimer = RESIZE_DEBOUNCE_FRAMES;
  }
  if (resizeTimer > 0) {
    resizeTimer--;

    if (resizeTimer == 0) {
      int currentW = GetScreenWidth();
      int currentH = GetScreenHeight();
      if (currentW != s_lastLoggedW || currentH != s_lastLoggedH) {
        s_lastLoggedW = currentW;
        s_lastLoggedH = currentH;

        CalculateInternal(static_cast<float>(GetScreenWidth()),
                          static_cast<float>(GetScreenHeight()));
        s_didResizeThisFrame = true;
        Log(LOG_LVL_DEBUG, "Viewport Resized [Debounced]");
      }
    }
  }
}
bool Viewport_wasResized(void) { return s_didResizeThisFrame; }
