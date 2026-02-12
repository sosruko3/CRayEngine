#include "cre_RendererCore.h"
#include "engine/loaders/cre_assetManager.h"
#include "engine/core/cre_logger.h"
#include "raylib.h"
#include "engine/platform/cre_viewport.h"
#include "engine/core/cre_typesMacro.h"
#include "engine/core/cre_colors.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

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
    ClearBackground(R_COL(creBLACK));
    
    /* Begin rendering to virtual canvas */
    BeginTextureMode(state.canvas);
    ClearBackground(R_COL(creBLACK));
}

void cre_RendererCore_EndFrame(void) {
    EndTextureMode();
    
    /* Upscale virtual canvas to physical window (no global flip) */
    creRectangle srcRect = {
        0.0f,
        0.0f,
        (float)state.canvas.texture.width,
        -(float)state.canvas.texture.height  /* reads "downwards" */
    };
    
    creRectangle destRect = {
        0.0f,
        0.0f,
        (float)GetScreenWidth(),
        (float)GetScreenHeight()
    };
    
    DrawTexturePro(state.canvas.texture, R_REC(srcRect), R_REC(destRect), (Vector2){0, 0}, 0.0f, R_COL(creBLANK));
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
void cre_RendererCore_DrawSprite(uint32_t spriteID, creVec2 position, creVec2 size, creVec2 pivot,
                            float rotation, bool flipX, bool flipY, creColor tint) {
    creRectangle srcRect = Asset_getRect((int)spriteID);
    
    float srcW = flipX ? -srcRect.width  : srcRect.width;
    float srcH = flipY ? -srcRect.height : srcRect.height;  /* Inverted: default flips for RT */
    
    creRectangle src = {
        srcRect.x,
        srcRect.y,
        srcW,
        srcH
    };
    
    creRectangle dest = {
        position.x,
        position.y,
        size.x,
        size.y
    };
    
    /* Pivot: normalized (0-1) -> pixel offset */
    creVec2 origin = {
        size.x * pivot.x,
        size.y * pivot.y
    };
    
    DrawTexturePro(
        state.cachedAtlas, R_REC(src), R_REC(dest), R_VEC(origin), rotation, 
        R_COL(tint));
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
