#include "cre_rendererCore.h"
#include "engine/loaders/cre_assetManager.h"
#include "engine/core/cre_logger.h"
#include "raylib.h"
#include "engine/platform/cre_viewport.h"
#include "engine/core/cre_typesMacro.h"
#include "engine/core/cre_colors.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <stddef.h>

/* ─────────────────────────────────────────────────────────────────────────────
 * Internal State (Encapsulated)
 * ───────────────────────────────────────────────────────────────────────────── */
typedef struct {
    RenderTexture2D canvas;
    Texture2D       cachedAtlas;
    Texture2D       currentTexture;
    Shader          currentShader;
    int32_t         currentBlendMode;
    int32_t         currentFilterMode;
    int32_t         virtualWidth;
    int32_t         virtualHeight;
} cre_RendererCore_State;

static cre_RendererCore_State state = {0};

/* ─────────────────────────────────────────────────────────────────────────────
 * Internal Helpers
 * ───────────────────────────────────────────────────────────────────────────── */
void rendererCore_RecreateCanvas(int32_t virtualWidth, int32_t virtualHeight) {
    if (state.canvas.id != 0) {
        UnloadRenderTexture(state.canvas);
    }
    state.canvas = LoadRenderTexture(virtualWidth, virtualHeight);
    SetTextureFilter(state.canvas.texture, TEXTURE_FILTER_POINT);
    state.virtualWidth  = virtualWidth;
    state.virtualHeight = virtualHeight;
}
/* ─────────────────────────────────────────────────────────────────────────────
 * Lifecycle
 * ───────────────────────────────────────────────────────────────────────────── */
void rendererCore_Init(int32_t virtualWidth,int32_t virtualHeight) {
    state.currentBlendMode = BLEND_ALPHA;
    state.currentFilterMode = -1;
    state.currentShader = (Shader){0};
    state.currentTexture = (Texture2D){0};
    rendererCore_RecreateCanvas(virtualWidth,virtualHeight);
    Log(LOG_LVL_INFO, "RENDERER: Initialized (%dx%d)", state.virtualWidth, state.virtualHeight);
}

void rendererCore_Shutdown(void) {
    if (state.canvas.id != 0) {
        UnloadRenderTexture(state.canvas);
        state.canvas = (RenderTexture2D){0};
    }
    state.cachedAtlas = (Texture2D){0};
    state.currentTexture = (Texture2D){0};
    state.currentShader = (Shader){0};
    state.currentBlendMode = BLEND_ALPHA;
    state.currentFilterMode = -1;
    Log(LOG_LVL_INFO, "RENDERER: Shutdown complete");
}

/* ─────────────────────────────────────────────────────────────────────────────
 * Frame Control
 * ───────────────────────────────────────────────────────────────────────────── */
void rendererCore_BeginFrame(void) {
    /* Cache atlas once per frame */
    state.cachedAtlas = Asset_getTexture();
    state.currentTexture = state.cachedAtlas;
    state.currentShader = (Shader){0};
    state.currentBlendMode = BLEND_ALPHA;
    
    BeginDrawing();
    ClearBackground(R_COL(creBLACK));
    
    /* Begin rendering to virtual canvas */
    BeginTextureMode(state.canvas);
    ClearBackground(R_COL(creBLACK));
}

void rendererCore_EndWorldRender(void) {
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
}
void rendererCore_EndFrame(void) {
    EndDrawing();
}

/* ─────────────────────────────────────────────────────────────────────────────
 * Camera Interface
 * ───────────────────────────────────────────────────────────────────────────── */
void rendererCore_BeginWorldMode(Camera2D camera) {
    BeginMode2D(camera);
}

void rendererCore_EndWorldMode(void) {
    EndMode2D();
}

/* ─────────────────────────────────────────────────────────────────────────────
 * Consolidated Sprite Draw
 * ───────────────────────────────────────────────────────────────────────────── */
void rendererCore_DrawSprite(uint32_t spriteID, creVec2 position, creVec2 size, creVec2 pivot,
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

    DrawTexturePro(state.currentTexture, R_REC(src), R_REC(dest), R_VEC(origin), rotation, R_COL(tint));
}

void rendererCore_SetState(Texture2D* texture, Shader* shader, int32_t blendMode, int32_t filterMode) {
    EndShaderMode();
    EndBlendMode();

    // This is in case current and next atlas has same filter. It would have been a problem for checks.
    Texture2D nextTexture = (texture != NULL) ? *texture :state.cachedAtlas;
    bool textureChanged = (nextTexture.id != state.currentTexture.id);
    state.currentTexture = nextTexture;

    if (state.currentTexture.id != 0 && (textureChanged ||state.currentFilterMode != filterMode)) {
        state.currentFilterMode = filterMode;
        SetTextureFilter(state.currentTexture, state.currentFilterMode);
    }
    
    state.currentShader = (shader != NULL && shader->id != 0) ? *shader : (Shader){0, NULL};
    state.currentBlendMode = blendMode;

    BeginBlendMode(state.currentBlendMode);
    if (state.currentShader.id != 0 ) {
        BeginShaderMode(state.currentShader);
    }
}

/*
* Helper function for RenderSystem
*/
void rendererCore_EndBatch(void) {
    EndShaderMode();
    EndBlendMode();

    state.currentShader = (Shader){0};
    state.currentBlendMode = BLEND_ALPHA;
}