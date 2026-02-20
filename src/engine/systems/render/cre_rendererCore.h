#ifndef CRE_RENDERERCORE_H
#define CRE_RENDERERCORE_H

#include "engine/core/cre_types.h"
#include "raylib.h"  // For Camera2D
#include <stdint.h>
#include <stdbool.h>

/* ─────────────────────────────────────────────────────────────────────────────
 * 2D Renderer Pipeline (Extend Strategy)
 * ─────────────────────────────────────────────────────────────────────────────
 * High-performance virtual canvas with fixed height and dynamic width.
 * Debounced resize to avoid RenderTexture thrashing. Atlas cached per frame.
 * Internal state encapsulated in cre_rendererCore.c.
 * ───────────────────────────────────────────────────────────────────────────── */

/* Helper functions*/
void rendererCore_RecreateCanvas(int32_t virtualWidth, int32_t virtualHeight);
//void rendererCore_ClearBackground(creColor color);
/* Lifecycle */
void rendererCore_Init(int32_t virtualWidth,int32_t virtualHeight);
void rendererCore_Shutdown(void);

/* Frame control */
void rendererCore_BeginFrame(void);
void rendererCore_EndWorldRender(void);
void rendererCore_EndFrame(void);

/* Camera interface (pass-through to Raylib Mode2D) */
void rendererCore_BeginWorldMode(Camera2D camera);
void rendererCore_EndWorldMode(void);

/* Consolidated sprite draw
 * - pivot: normalized (0,0)=top-left, (0.5,0.5)=center, (1,1)=bottom-right */
void rendererCore_DrawSprite(uint32_t spriteID, creVec2 position, creVec2 size, creVec2 pivot,
                            float rotation,bool flipX, bool flipY, creColor tint);

void rendererCore_SetState(Texture2D* texture, Shader* shader, int32_t blendMode, int32_t filterMode);

/* Helper function for RenderSystem*/
void rendererCore_EndBatch(void);
#endif