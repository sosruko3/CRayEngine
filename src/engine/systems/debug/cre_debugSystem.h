/**
 * @file cre_debugSystem.h
 * @brief Physics Insight Debug Alarms + Stats HUD
 *
 * Professional developer tools for visualizing physics engine internals.
 * Designed for high-performance (16,000+ entities) without allocations.
 *
 * Visualization:
 *   1. Alarm Overlay - Data corruption and orphan entity detection
 *   2. Stats HUD     - Real-time ECS and frame telemetry
 *
 * Controls:
 *   F1        - Toggle debug overlay on/off
 *   TAB       - Toggle stats HUD (always available)
 */
#ifndef DEBUGSYSTEM_H
#define DEBUGSYSTEM_H

#include <stdbool.h>
#include <stdint.h>

struct EntityRegistry;
struct CommandBus;

// ============================================================================
// Visualization Mode Enumeration
// ============================================================================

typedef enum {
  DEBUG_MODE_OFF = 0,
  DEBUG_MODE_ALARMS,
  DEBUG_MODE_COUNT
} DebugVisualizationMode;

// ============================================================================
// Public API
// ============================================================================

/**
 * @brief Initialize debug system (call once at startup).
 */
void DebugSystem_Init(void);

/**
 * @brief Handle debug input.
 *
 * Key bindings:
 *   F1      - Toggle debug overlay
 *   TAB     - Toggle stats HUD
 *
 * @param reg Entity registry
 */
void DebugSystem_HandleInput(EntityRegistry &reg);

/**
 * @brief Get count of active entities.
 * @param reg Entity registry
 * @return Number of active entities
 */
uint32_t DebugSystem_GetActiveCount(EntityRegistry &reg);

/**
 * @brief Render world-space debug visualizations.
 *
 * Call this INSIDE BeginWorldMode/EndWorldMode for proper camera transform.
 * Renders: alarm overlay (NaN/Inf and orphan detection).
 *
 * @param reg Entity registry (read-only access)
 */
void DebugSystem_RenderWorldSpace(EntityRegistry &reg);

/**
 * @brief Render screen-space HUD elements.
 *
 * Call this AFTER EndWorldMode for proper screen positioning.
 * Renders: stats HUD.
 *
 * @param reg Entity registry (read-only access)
 */
void DebugSystem_RenderScreenSpace(EntityRegistry &reg);

/**
 * @brief Main entry point for physics debug (world-space only).
 *
 * For backward compatibility. Equivalent to DebugSystem_RenderWorldSpace().
 * Call inside BeginWorldMode/EndWorldMode.
 *
 * @param reg Entity registry (read-only access)
 */
void DebugSystem_RenderPhysicsInsight(EntityRegistry &reg);

/**
 * @brief Render stats HUD overlay (can be called independently).
 *
 * Shows: Entity counts, physics timing, memory usage.
 *
 * @param reg Entity registry
 */
void DebugSystem_RenderStatsHUD(EntityRegistry &reg);

/**
 * @brief Get current visualization mode.
 * @return Current mode enum value
 */
DebugVisualizationMode DebugSystem_GetMode(void);

/**
 * @brief Set visualization mode directly.
 * @param mode Mode to activate
 */
void DebugSystem_SetMode(DebugVisualizationMode mode);

/**
 * @brief Check if debug overlay is enabled.
 * @return true if overlay is visible
 */
bool DebugSystem_IsEnabled(void);

/**
 * @brief Legacy draw function (placeholder).
 */
void DebugSystem_Draw(void);

#endif // DEBUGSYSTEM_H
