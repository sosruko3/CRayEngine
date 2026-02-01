#include "cre_RendererCore.h"
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
} cre_RendererCore_State;

static cre_RendererCore_State state = {0};

/* ─────────────────────────────────────────────────────────────────────────────
 * Internal Helpers
 * ───────────────────────────────────────────────────────────────────────────── */
void cre_RendererCore_RecreateCanvas(int virtualWidth, int virtualHeight) {
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
void cre_RendererCore_Init(int virtualWidth,int virtualHeight) {
    state.filterMode      = TEXTURE_FILTER_POINT; // FOR PIXEL ARTS.
    cre_RendererCore_RecreateCanvas(virtualWidth,virtualHeight);
    Log(LOG_LVL_INFO, "RENDERER: Initialized (%dx%d)", state.virtualWidth, state.virtualHeight);
}

void cre_RendererCore_Shutdown(void) {
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
void cre_RendererCore_BeginFrame(void) {
    /* Cache atlas once per frame */
    state.cachedAtlas = Asset_getTexture();
    
    BeginDrawing();
    ClearBackground(BLACK);
    
    /* Begin rendering to virtual canvas */
    BeginTextureMode(state.canvas);
    ClearBackground(BLACK);
}

void cre_RendererCore_EndFrame(void) {
    EndTextureMode();
    
    /* Upscale virtual canvas to physical window (no global flip) */
    Rectangle srcRect = {
        0.0f,
        0.0f,
        (float)state.canvas.texture.width,
        -(float)state.canvas.texture.height  /* reads "downwards" */
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
void cre_RendererCore_BeginWorldMode(Camera2D camera) {
    BeginMode2D(camera);
}

void cre_RendererCore_EndWorldMode(void) {
    EndMode2D();
}

/* ─────────────────────────────────────────────────────────────────────────────
 * Consolidated Sprite Draw
 * ───────────────────────────────────────────────────────────────────────────── */
void cre_RendererCore_DrawSprite(uint32_t spriteID, Vector2 position,Vector2 size ,Vector2 pivot,
                            float rotation, bool flipX, bool flipY, Color tint) {
    Rectangle srcRect = Asset_getRect((int)spriteID);
    
    float srcW = flipX ? -srcRect.width  : srcRect.width;
    float srcH = flipY ? -srcRect.height : srcRect.height;  /* Inverted: default flips for RT */
    
    Rectangle src = {
        srcRect.x,
        srcRect.y,
        srcW,
        srcH
    };
    
    Rectangle dest = {
        position.x,
        position.y,
        size.x,
        size.y
    };
    
    /* Pivot: normalized (0-1) -> pixel offset */
    Vector2 origin = {
        size.x * pivot.x,
        size.y * pivot.y
    };
    
    DrawTexturePro(state.cachedAtlas, src, dest, origin, rotation, tint);
}

/* ─────────────────────────────────────────────────────────────────────────────
 * Settings
 * ───────────────────────────────────────────────────────────────────────────── */
void cre_RendererCore_SetFilter(int filterMode) {
    state.filterMode = filterMode;
    if (state.canvas.id != 0) {
        SetTextureFilter(state.canvas.texture, filterMode);
    }
}
