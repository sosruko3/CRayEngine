#include "cre_renderer.h"
#include "asset_manager.h"
#include "logger.h"
#include "raylib.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "viewport.h"

/* ─────────────────────────────────────────────────────────────────────────────
 * Internal State (Encapsulated)
 * ───────────────────────────────────────────────────────────────────────────── */
typedef struct {
    RenderTexture2D canvas;
    Texture2D       cachedAtlas;
    int             virtualWidth;
    int             virtualHeight;
    int             filterMode;
} creRenderer_State;

static creRenderer_State state = {0};

/* ─────────────────────────────────────────────────────────────────────────────
 * Internal Helpers
 * ───────────────────────────────────────────────────────────────────────────── */
static void RecreateCanvas(int virtualWidth, int virtualHeight) {
    if (state.canvas.id != 0) {
        UnloadRenderTexture(state.canvas);
    }
    state.canvas = LoadRenderTexture(virtualWidth, virtualHeight);
    SetTextureFilter(state.canvas.texture, state.filterMode);
    state.virtualWidth  = virtualWidth;
    state.virtualHeight = virtualHeight;
}
/* ─────────────────────────────────────────────────────────────────────────────
 * Lifecycle
 * ───────────────────────────────────────────────────────────────────────────── */
void creRenderer_Init(int virtualWidth,int virtualHeight) {
    state.filterMode      = TEXTURE_FILTER_BILINEAR;
    RecreateCanvas(virtualWidth,virtualHeight);
    Log(LOG_LVL_INFO, "RENDERER: Initialized (%dx%d)", state.virtualWidth, state.virtualHeight);
}

void creRenderer_Shutdown(void) {
    if (state.canvas.id != 0) {
        UnloadRenderTexture(state.canvas);
        state.canvas = (RenderTexture2D){0};
    }
    state.cachedAtlas = (Texture2D){0};
    Log(LOG_LVL_INFO, "RENDERER: Shutdown complete");
}

/* ─────────────────────────────────────────────────────────────────────────────
 * Frame Control
 * ───────────────────────────────────────────────────────────────────────────── */
void creRenderer_BeginFrame(void) {
    /* Cache atlas once per frame */
    state.cachedAtlas = Asset_getTexture();
    
    /* Debounced resize handling (Extend strategy) */
    //SyncInternalResolution();
    
    BeginDrawing();
    ClearBackground(BLACK);
    
    /* Begin rendering to virtual canvas */
    BeginTextureMode(state.canvas);
    ClearBackground(BLACK);
}

void creRenderer_EndFrame(void) {
    EndTextureMode();
    
    /* Upscale virtual canvas to physical window (no global flip) */
    Rectangle srcRect = {
        0.0f,
        (float)state.canvas.texture.height,
        (float)state.canvas.texture.width,
        -(float)state.canvas.texture.height  /* Flip Y: sample from bottom-up */
    };
    
    Rectangle destRect = {
        0.0f,
        0.0f,
        (float)GetScreenWidth(),
        (float)GetScreenHeight()
    };
    
    DrawTexturePro(state.canvas.texture, srcRect, destRect, (Vector2){0, 0}, 0.0f, WHITE);
    EndDrawing();
}

/* ─────────────────────────────────────────────────────────────────────────────
 * Camera Interface
 * ───────────────────────────────────────────────────────────────────────────── */
void creRenderer_BeginWorldMode(Camera2D camera) {
    BeginMode2D(camera);
}

void creRenderer_EndWorldMode(void) {
    EndMode2D();
}

/* ─────────────────────────────────────────────────────────────────────────────
 * Consolidated Sprite Draw
 * ───────────────────────────────────────────────────────────────────────────── */
void creRenderer_DrawSprite(uint32_t spriteID, Vector2 position, Vector2 pivot,
                            float rotation, float scale, bool flipX, bool flipY, Color tint) {
    Rectangle srcRect = Asset_getRect((int)spriteID);
    
    /* Base Y-flip for RenderTexture correction (OpenGL Y-up) */
    /* Then apply user flip on top */
    float srcW = flipX ? -srcRect.width  : srcRect.width;
    float srcH = flipY ?  srcRect.height : -srcRect.height;  /* Inverted: default flips for RT */
    
    Rectangle src = {
        srcRect.x,
        srcRect.y,
        srcW,
        srcH
    };
    
    float destW = fabsf(srcRect.width)  * scale;
    float destH = fabsf(srcRect.height) * scale;
    
    Rectangle dest = {
        position.x,
        position.y,
        destW,
        destH
    };
    
    /* Pivot: normalized (0-1) -> pixel offset */
    Vector2 origin = {
        destW * pivot.x,
        destH * pivot.y
    };
    
    DrawTexturePro(state.cachedAtlas, src, dest, origin, rotation, tint);
}

/* ─────────────────────────────────────────────────────────────────────────────
 * Settings
 * ───────────────────────────────────────────────────────────────────────────── */
void creRenderer_SetFilter(int filterMode) {
    state.filterMode = filterMode;
    if (state.canvas.id != 0) {
        SetTextureFilter(state.canvas.texture, filterMode);
    }
}
