#ifndef CRE_RENDERERCORE_H
#define CRE_RENDERERCORE_H

#include "raylib.h"
#include <stdint.h>
#include <stdbool.h>

/* ─────────────────────────────────────────────────────────────────────────────
 * 2D Renderer Pipeline (Extend Strategy)
 * ─────────────────────────────────────────────────────────────────────────────
 * High-performance virtual canvas with fixed height and dynamic width.
 * Debounced resize to avoid RenderTexture thrashing. Atlas cached per frame.
 * Internal state encapsulated in cre_RendererCore.c.
 * ───────────────────────────────────────────────────────────────────────────── */

/* Lifecycle */
void cre_RendererCore_Init(int virtualWidth,int virtualHeight);
void cre_RendererCore_RecreateCanvas(int virtualWidth, int virtualHeight);
void cre_RendererCore_Shutdown(void);

/* Frame control */
void cre_RendererCore_BeginFrame(void);
void cre_RendererCore_EndFrame(void);

/* Camera interface (pass-through to Raylib Mode2D) */
void cre_RendererCore_BeginWorldMode(Camera2D camera);
void cre_RendererCore_EndWorldMode(void);

/* Consolidated sprite draw
 * - pivot: normalized (0,0)=top-left, (0.5,0.5)=center, (1,1)=bottom-right
 * - flipX/flipY: handled via negative srcRect dimensions */
void cre_RendererCore_DrawSprite(uint32_t spriteID, Vector2 position,Vector2 size, Vector2 pivot,
                            float rotation,bool flipX, bool flipY, Color tint);

/* Settings */
void cre_RendererCore_SetFilter(int filterMode);

#endif