/**
 * @file cre_debugSystem.h
 * @brief Physics Insight Debug Visualization Suite
 * 
 * Professional developer tools for visualizing physics engine internals.
 * Designed for high-performance (16,000+ entities) without allocations.
 * 
 * Visualization Modes:
 *   1. Spatial Hash Heatmap  - Cell density hotspots
 *   2. Entity State Overlay  - Sleep/Wake/Culled/Static states
 *   3. Velocity Field        - Momentum vectors and energy
 *   4. Collision Layers      - Layer/Mask group visualization
 *   5. Stats HUD             - Real-time performance metrics
 * 
 * Controls:
 *   F1        - Toggle debug overlay on/off
 *   F2-F5     - Switch visualization modes (1-4)
 *   F8        - Cycle through modes
 *   TAB       - Toggle stats HUD (always available)
 */
#ifndef DEBUGSYSTEM_H
#define DEBUGSYSTEM_H

#include <stdint.h>
#include <stdbool.h>

typedef struct EntityRegistry EntityRegistry;
typedef struct CommandBus CommandBus;

// ============================================================================
// Visualization Mode Enumeration
// ============================================================================

typedef enum {
    DEBUG_MODE_OFF = 0,
    DEBUG_MODE_SPATIAL_HASH,    // 1: Cell density heatmap
    DEBUG_MODE_ENTITY_STATE,    // 2: Sleep/Wake/Culled overlay
    DEBUG_MODE_VELOCITY_FIELD,  // 3: Velocity vectors + energy
    DEBUG_MODE_COLLISION_LAYERS,// 4: Layer/Mask groups
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
 * @brief Handle debug input (mode switching, spawning).
 * 
 * Key bindings:
 *   F1      - Toggle debug overlay
 *   F2-F5   - Select visualization mode
 *   F8      - Cycle modes
 *   TAB     - Toggle stats HUD
 * 
 * @param reg Entity registry
 */
void DebugSystem_HandleInput(EntityRegistry* reg);

/**
 * @brief Get count of active entities.
 * @param reg Entity registry
 * @return Number of active entities
 */
uint32_t DebugSystem_GetActiveCount(EntityRegistry* reg);

/**
 * @brief Render world-space debug visualizations.
 * 
 * Call this INSIDE BeginWorldMode/EndWorldMode for proper camera transform.
 * Renders: entity states, velocity vectors, spatial hash overlay, etc.
 * 
 * @param reg Entity registry (read-only access)
 */
void DebugSystem_RenderWorldSpace(EntityRegistry* reg);

/**
 * @brief Render screen-space HUD elements.
 * 
 * Call this AFTER EndWorldMode for proper screen positioning.
 * Renders: stats HUD, mode indicator, legends.
 * 
 * @param reg Entity registry (read-only access)
 */
void DebugSystem_RenderScreenSpace(EntityRegistry* reg);

/**
 * @brief Main entry point for physics debug (world-space only).
 * 
 * For backward compatibility. Equivalent to DebugSystem_RenderWorldSpace().
 * Call inside BeginWorldMode/EndWorldMode.
 * 
 * @param reg Entity registry (read-only access)
 */
void DebugSystem_RenderPhysicsInsight(EntityRegistry* reg);

/**
 * @brief Render stats HUD overlay (can be called independently).
 * 
 * Shows: Entity counts, physics timing, memory usage.
 * 
 * @param reg Entity registry
 */
void DebugSystem_RenderStatsHUD(EntityRegistry* reg);

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
/**
 * @brief Legacy draw function (placeholder).
 */
void DebugSystem_Draw(void);

/**
 * @brief Render mouse hover tooltip for entity inspection.
 * 
 * Shows entity details (ID, position, velocity, flags) when mouse
 * hovers over an entity's AABB. Call in screen-space after EndWorldMode.
 * 
 * @param reg Entity registry (read-only access)
 */
void DebugSystem_RenderMouseHover(EntityRegistry* reg);

#endif // DEBUGSYSTEM_H