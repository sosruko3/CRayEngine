#ifndef CRE_RENDERER_H
#define CRE_RENDERER_H

#include "raylib.h"
#include <stdint.h>
#include <stdbool.h>

/* ─────────────────────────────────────────────────────────────────────────────
 * 2D Renderer Pipeline (Extend Strategy)
 * ─────────────────────────────────────────────────────────────────────────────
 * High-performance virtual canvas with fixed height and dynamic width.
 * Debounced resize to avoid RenderTexture thrashing. Atlas cached per frame.
 * Internal state encapsulated in cre_renderer.c.
 * ───────────────────────────────────────────────────────────────────────────── */

/* Lifecycle */
void creRenderer_Init(int virtualHeight);
void creRenderer_Shutdown(void);

/* Frame control */
void creRenderer_BeginFrame(void);
void creRenderer_EndFrame(void);

/* Camera interface (pass-through to Raylib Mode2D) */
void creRenderer_BeginWorldMode(Camera2D camera);
void creRenderer_EndWorldMode(void);

/* Consolidated sprite draw
 * - pivot: normalized (0,0)=top-left, (0.5,0.5)=center, (1,1)=bottom-right
 * - flipX/flipY: handled via negative srcRect dimensions
 * - scale: uniform scale factor */
void creRenderer_DrawSprite(uint32_t spriteID, Vector2 position, Vector2 pivot,
                            float rotation, float scale, bool flipX, bool flipY, Color tint);

/* Settings */
void creRenderer_SetFilter(int filterMode);

/* Accessors */
int  creRenderer_GetVirtualWidth(void);
int  creRenderer_GetVirtualHeight(void);
void creRenderer_GetVirtualSize(int *outWidth, int *outHeight);

#endif