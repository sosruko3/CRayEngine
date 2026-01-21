#include "cre_renderer.h"
#include "asset_manager.h"
#include "logger.h"
#include "raylib.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/* ─────────────────────────────────────────────────────────────────────────────
 * Internal State (Encapsulated)
 * ───────────────────────────────────────────────────────────────────────────── */
#define RESIZE_DEBOUNCE_FRAMES 6  /* ~100ms at 60Hz, ~50ms at 120Hz */

typedef struct {
    RenderTexture2D canvas;
    Texture2D       cachedAtlas;
    int             virtualWidth;
    int             virtualHeight;
    int             filterMode;
    /* Debounce state */
    int             pendingWidth;
    int             pendingHeight;
    int             debounceCounter;
} creRenderer_State;

static creRenderer_State state = {0};

/* ─────────────────────────────────────────────────────────────────────────────
 * Internal Helpers
 * ───────────────────────────────────────────────────────────────────────────── */
static int CalcVirtualWidth(int virtualHeight) {
    float aspect = (float)GetScreenWidth() / (float)GetScreenHeight();
    return (int)(aspect * (float)virtualHeight);
}

static void RecreateCanvas(int virtualWidth, int virtualHeight) {
    if (state.canvas.id != 0) {
        UnloadRenderTexture(state.canvas);
    }
    state.canvas = LoadRenderTexture(virtualWidth, virtualHeight);
    SetTextureFilter(state.canvas.texture, state.filterMode);
    state.virtualWidth  = virtualWidth;
    state.virtualHeight = virtualHeight;
}

static void SyncInternalResolution(void) {
    int targetWidth = CalcVirtualWidth(state.virtualHeight);
    
    /* Check if dimensions changed */
    if (targetWidth == state.virtualWidth) {
        state.debounceCounter = 0;
        state.pendingWidth    = 0;
        return;
    }
    
    /* Start or continue debounce */
    if (state.pendingWidth != targetWidth) {
        state.pendingWidth    = targetWidth;
        state.pendingHeight   = state.virtualHeight;
        state.debounceCounter = RESIZE_DEBOUNCE_FRAMES;
    } else if (state.debounceCounter > 0) {
        state.debounceCounter--;
        if (state.debounceCounter == 0) {
            RecreateCanvas(state.pendingWidth, state.pendingHeight);
            Log(LOG_LVL_INFO, "RENDERER: Canvas resized (%dx%d)", state.pendingWidth, state.pendingHeight);
            state.pendingWidth = 0;
        }
    }
}

/* ─────────────────────────────────────────────────────────────────────────────
 * Lifecycle
 * ───────────────────────────────────────────────────────────────────────────── */
void creRenderer_Init(int virtualHeight) {
    state.virtualHeight   = virtualHeight;
    state.virtualWidth    = CalcVirtualWidth(virtualHeight);
    state.filterMode      = TEXTURE_FILTER_BILINEAR;
    state.debounceCounter = 0;
    state.pendingWidth    = 0;
    state.pendingHeight   = 0;
    
    RecreateCanvas(state.virtualWidth, state.virtualHeight);
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
    SyncInternalResolution();
    
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

/* ─────────────────────────────────────────────────────────────────────────────
 * Accessors
 * ───────────────────────────────────────────────────────────────────────────── */
int creRenderer_GetVirtualWidth(void) {
    return state.virtualWidth;
}

int creRenderer_GetVirtualHeight(void) {
    return state.virtualHeight;
}

void creRenderer_GetVirtualSize(int *outWidth, int *outHeight) {
    if (outWidth)  *outWidth  = state.virtualWidth;
    if (outHeight) *outHeight = state.virtualHeight;
}