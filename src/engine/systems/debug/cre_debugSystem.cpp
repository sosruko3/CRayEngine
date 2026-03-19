/**
 * @file cre_debugSystem.cpp
 * @brief Physics Insight Debug Alarms + Stats HUD
 */
#include "cre_debugSystem.h"
#include "engine/core/cre_colors.h"
#include "engine/core/cre_config.h"
#include "engine/core/cre_logger.h"
#include "engine/core/cre_types.h"
#include "engine/core/cre_typesMacro.h"
#include "engine/ecs/cre_entityRegistry.h"
#include "engine/platform/cre_viewport.h"
#include "engine/systems/camera/cre_cameraUtils.h"
#include "raylib.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

// ============================================================================
// Static State
// ============================================================================

static bool s_debugEnabled = false;
static bool s_statsHudEnabled = true;

// Frame timing for stats
static double s_lastFrameTime = 0.0;
static double s_avgFrameTime = 0.0;

// ============================================================================
// Forward Declarations
// ============================================================================

static void DebugSystem_RenderAlarms(EntityRegistry *reg);

// ============================================================================
// Public API Implementation
// ============================================================================

void DebugSystem_Init(void) {
  s_debugEnabled = false;
  s_statsHudEnabled = true;
  Log(LOG_LVL_INFO,
      "Debug System Initialized - Press F1 to toggle alarms, TAB for stats");
}

void DebugSystem_HandleInput(EntityRegistry *reg) {
  assert(reg && "reg is NULL");
  (void)reg;

  if (IsKeyPressed(KEY_F1)) {
    s_debugEnabled = !s_debugEnabled;
    Log(LOG_LVL_INFO, "Debug Overlay: %s", s_debugEnabled ? "ON" : "OFF");
  }

  if (IsKeyPressed(KEY_TAB)) {
    s_statsHudEnabled = !s_statsHudEnabled;
  }
}

uint32_t DebugSystem_GetActiveCount(EntityRegistry *reg) {
  assert(reg && "reg is NULL");
  return reg->active_count;
}

void DebugSystem_RenderWorldSpace(EntityRegistry *reg) {
  assert(reg && "reg is NULL");

  if (!s_debugEnabled) {
    return;
  }

  DebugSystem_RenderAlarms(reg);
}

void DebugSystem_RenderPhysicsInsight(EntityRegistry *reg) {
  DebugSystem_RenderWorldSpace(reg);
}

void DebugSystem_RenderScreenSpace(EntityRegistry *reg) {
  assert(reg && "reg is NULL");

  if (s_statsHudEnabled) {
    DebugSystem_RenderStatsHUD(reg);
  }
}

void DebugSystem_RenderStatsHUD(EntityRegistry *reg) {
  assert(reg && "reg is NULL");

  // Calculate frame time
  const double currentTime = GetTime();
  const double frameTime = (currentTime - s_lastFrameTime) * 1000.0;
  s_lastFrameTime = currentTime;
  s_avgFrameTime = s_avgFrameTime * 0.95 + frameTime * 0.05;

  // Count entity states
  uint32_t activeCount = 0;
  uint32_t sleepingCount = 0;
  uint32_t staticCount = 0;
  uint32_t culledCount = 0;
  uint32_t physicsCount = 0;
  uint32_t awakeCount = 0;

  const uint32_t bound = reg->max_used_bound;
  for (uint32_t i = 0; i < bound; i++) {
    const uint64_t flags = reg->state_flags[i];
    const uint64_t comps = reg->component_masks[i];

    if (!(flags & FLAG_ACTIVE)) {
      continue;
    }
    activeCount++;

    if (flags & FLAG_SLEEPING) {
      sleepingCount++;
    }
    if (flags & FLAG_STATIC) {
      staticCount++;
    }
    if (flags & FLAG_CULLED) {
      culledCount++;
    }
    if (comps & COMP_PHYSICS) {
      physicsCount++;
    }

    // Awake = active AND not sleeping AND not culled
    if (!(flags & (FLAG_SLEEPING | FLAG_CULLED))) {
      awakeCount++;
    }
  }

  // Draw HUD background
  const int hudX = 10;
  const int hudY = 10;
  const int hudWidth = 280;
  const int hudHeight = 180;

  DrawRectangle(hudX, hudY, hudWidth, hudHeight, Color{20, 20, 30, 220});
  DrawRectangleLines(hudX, hudY, hudWidth, hudHeight, Color{80, 80, 100, 255});

  // Title
  DrawText("PHYSICS INSIGHT", hudX + 10, hudY + 8, 16,
           Color{100, 200, 255, 255});
  DrawLine(hudX + 5, hudY + 28, hudX + hudWidth - 5, hudY + 28,
           Color{60, 60, 80, 255});

  int rowY = hudY + 35;
  constexpr int rowSpacing = 18;

  char buffer[128];

  const creColor fpsColor = (s_avgFrameTime < 16.67) ? creColor{0, 228, 48, 255}
                            : (s_avgFrameTime < 33.33)
                                ? creColor{253, 249, 0, 255}
                                : creColor{230, 41, 55, 255};

  snprintf(buffer, sizeof(buffer), "Frame: %.2f ms (%.0f FPS)", s_avgFrameTime,
           1000.0 / s_avgFrameTime);
  DrawText(buffer, hudX + 10, rowY, 14,
           Color{fpsColor.r, fpsColor.g, fpsColor.b, fpsColor.a});
  rowY += rowSpacing;

  snprintf(buffer, sizeof(buffer), "Entities: %u / %u", activeCount,
           MAX_ENTITIES);
  DrawText(buffer, hudX + 10, rowY, 14, WHITE);
  rowY += rowSpacing;

  snprintf(buffer, sizeof(buffer), "Physics:  %u", physicsCount);
  DrawText(buffer, hudX + 10, rowY, 14, Color{150, 200, 255, 255});
  rowY += rowSpacing;

  snprintf(buffer, sizeof(buffer), "Awake:    %u", awakeCount);
  DrawText(buffer, hudX + 10, rowY, 14, GREEN);
  rowY += rowSpacing;

  snprintf(buffer, sizeof(buffer), "Sleeping: %u", sleepingCount);
  DrawText(buffer, hudX + 10, rowY, 14, YELLOW);
  rowY += rowSpacing;

  snprintf(buffer, sizeof(buffer), "Static:   %u", staticCount);
  DrawText(buffer, hudX + 10, rowY, 14, BLUE);
  rowY += rowSpacing;

  snprintf(buffer, sizeof(buffer), "Culled:   %u", culledCount);
  DrawText(buffer, hudX + 10, rowY, 14, RED);
}

DebugVisualizationMode DebugSystem_GetMode(void) {
  return s_debugEnabled ? DEBUG_MODE_ALARMS : DEBUG_MODE_OFF;
}

void DebugSystem_SetMode(DebugVisualizationMode mode) {
  s_debugEnabled = (mode != DEBUG_MODE_OFF);
}

bool DebugSystem_IsEnabled(void) { return s_debugEnabled; }

void DebugSystem_Draw(void) {
  // Legacy placeholder - use DebugSystem_RenderPhysicsInsight instead
}

// ============================================================================
// World-Space Alarm Overlay
// ============================================================================

static void DebugSystem_RenderAlarms(EntityRegistry *reg) {
  const uint32_t bound = reg->max_used_bound;

  const creColor colorNaN = {255, 0, 0, 255};
  const creColor colorOrphan = creORANGE;

  constexpr float ORPHAN_THRESHOLD = 10000.0f;

  const ViewportSize vp = Viewport_Get();
  const Camera2D cam = cameraUtils_GetActiveRaylib(reg, vp);

  for (uint32_t i = 0; i < bound; i++) {
    const uint64_t flags = reg->state_flags[i];
    if (!(flags & FLAG_ACTIVE)) {
      continue;
    }

    const float px = reg->pos_x[i];
    const float py = reg->pos_y[i];

    // Data corruption alarm: NaN/Inf transform values.
    if (isnan(px) || isnan(py) || isinf(px) || isinf(py)) {
      const creVec2 camPos = {cam.target.x, cam.target.y};
      constexpr float boxSize = 100.0f;
      const float halfBoxSize = boxSize * 0.5f;

      DrawRectangle(static_cast<int>(camPos.x - halfBoxSize),
                    static_cast<int>(camPos.y - halfBoxSize),
                    static_cast<int>(boxSize), static_cast<int>(boxSize),
                    Color{255, 0, 0, 100});

      DrawRectangleLines(static_cast<int>(camPos.x - halfBoxSize),
                         static_cast<int>(camPos.y - halfBoxSize),
                         static_cast<int>(boxSize), static_cast<int>(boxSize),
                         R_COL(colorNaN));

      char nanStr[64];
      snprintf(nanStr, sizeof(nanStr), "NaN ERROR [ID:%u]", i);
      DrawText(nanStr, static_cast<int>(camPos.x - 60.0f),
               static_cast<int>(camPos.y - 10.0f), 16, R_COL(colorNaN));
      continue;
    }

    // Orphan alarm: entity is far outside expected world bounds.
    if (px < -ORPHAN_THRESHOLD || px > ORPHAN_THRESHOLD ||
        py < -ORPHAN_THRESHOLD || py > ORPHAN_THRESHOLD) {
      const creVec2 camPos = {cam.target.x, cam.target.y};
      const float dx = px - camPos.x;
      const float dy = py - camPos.y;
      const float dist = sqrtf(dx * dx + dy * dy);

      if (dist > 1.0f) {
        const float nx = dx / dist;
        const float ny = dy / dist;

        const float edgeX = camPos.x + nx * (vp.width / cam.zoom / 2.5f);
        const float edgeY = camPos.y + ny * (vp.height / cam.zoom / 2.5f);

        DrawCircle(static_cast<int>(edgeX), static_cast<int>(edgeY), 8.0f,
                   R_COL(colorOrphan));
        DrawLine(static_cast<int>(edgeX), static_cast<int>(edgeY),
                 static_cast<int>(edgeX + nx * 20.0f),
                 static_cast<int>(edgeY + ny * 20.0f), R_COL(colorOrphan));

        char orphanStr[64];
        snprintf(orphanStr, sizeof(orphanStr), "ORPHAN [ID:%u] @%.0f,%.0f", i,
                 static_cast<double>(px), static_cast<double>(py));
        DrawText(orphanStr, static_cast<int>(edgeX - 50.0f),
                 static_cast<int>(edgeY - 25.0f), 10, R_COL(colorOrphan));
      }
    }
  }
}
