/**
 * @file cre_debugSystem.c
 * @brief Physics Insight Debug Visualization Suite
 * 
 * Professional visualization tools for debugging high-performance physics.
 * Zero allocations in render path. Static buffers for all temporary data.
 * 
 * Architecture:
 *   - Mode switching via F-keys
 *   - Each mode has dedicated render function
 *   - Stats HUD available in all modes (TAB toggle)
 *   - Alpha blending for non-intrusive overlays
 */
#include "cre_debugSystem.h"
#include "raylib.h"
#include "engine/core/cre_types.h"
#include "engine/core/cre_typesMacro.h"
#include "engine/core/cre_colors.h"
#include "game/entity_types.h"
#include "engine/ecs/cre_entityManager.h"
#include "engine/ecs/cre_entityRegistry.h"
#include "engine/core/cre_logger.h"
#include "engine/systems/animation/cre_animationSystem.h"
#include "engine/platform/cre_viewport.h"
#include "engine/systems/camera/cre_cameraSystem.h"
#include "engine/systems/camera/cre_cameraUtils.h"
#include "engine/core/cre_config.h"
#include "atlas_data.h"
#include "engine/core/cre_commandBus.h"
#include "engine/systems/physics/cre_physics_defs.h"
#include "engine/systems/physics/cre_physicsSystem.h"
#include "engine/systems/physics/cre_spatialHash.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

// ============================================================================
// Configuration Constants
// ============================================================================

#define SPAWN_COUNT 500
#define SCALE_FACTOR 2.0f

// Visualization settings
#define VELOCITY_SCALE 0.5f          // Scale factor for velocity vectors
#define MAX_VELOCITY_DISPLAY 200.0f  // Clamp velocity vector length
#define CONTACT_DECAY_RATE 0.95f     // Contact pressure fade per frame
#define HEATMAP_CELL_ALPHA 150       // Alpha for heatmap cells
#define ENTITY_DOT_RADIUS 3          // Radius for entity state dots

// Spatial hash visualization (matches cre_config.h)
#define VIS_GRID_SIZE SPATIAL_GRID_SIZE
#define VIS_HASH_SIZE SPATIAL_HASH_SIZE

// ============================================================================
// Static State (No Allocations in Render Path)
// ============================================================================

static DebugVisualizationMode s_currentMode = DEBUG_MODE_OFF;
static bool s_debugEnabled = false;
static bool s_statsHudEnabled = true;

// Frame timing for stats
static double s_lastFrameTime = 0.0;
static double s_avgFrameTime = 0.0;
static double s_physicsTime = 0.0;

// Cached data for legends (computed in world-space, drawn in screen-space)
static uint16_t s_heatmapMaxCount = 1;
static float s_velocityMaxSpeed = 0.0f;
static float s_velocityAvgSpeed = 0.0f;
static uint32_t s_velocityMovingCount = 0;
static uint32_t s_layerCounts[8] = {0};

// Mode names for HUD display
static const char* s_modeNames[] = {
    "OFF",
    "Spatial Hash Heatmap",
    "Entity State Overlay",
    "Velocity Field",
    "Collision Layers"
};

// Layer colors for visualization
static const creColor s_layerColors[] = {
    creRED,  // L_PLAYER - Red
    creGREEN,  // L_ENEMY - Green
    creBLUE,  // L_BULLET - Blue
    creYELLOW,  // L_WORLD - Yellow
    creLAVENDER,  // L_PICKUP - Lavender
    creDARKBLUE,  // L_TRIGGER - Dark Blue
    creDARKGREY,  // Default - Gray
    creORANGE,  // Extra - Orange
};

// ============================================================================
// Color Utility Functions
// ============================================================================

/**
 * @brief Interpolate between two colors.
 */
static creColor LerpColor(creColor a, creColor b, float t) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return (creColor){
        (unsigned char)(a.r + (b.r - a.r) * t),
        (unsigned char)(a.g + (b.g - a.g) * t),
        (unsigned char)(a.b + (b.b - a.b) * t),
        (unsigned char)(a.a + (b.a - a.a) * t)
    };
}

/**
 * @brief Get heatmap color from cold (blue) to hot (red).
 * @param value Normalized value 0.0 to 1.0
 * @param alpha Alpha value
 */
static creColor GetHeatmapColor(float value, unsigned char alpha) {
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;
    
    creColor result;
    if (value < 0.25f) {
        // Blue to Cyan
        float t = value / 0.25f;
        result = (creColor){ 0, (unsigned char)(t * 255), 255, alpha };
    } else if (value < 0.5f) {
        // Cyan to Green
        float t = (value - 0.25f) / 0.25f;
        result = (creColor){ 0, 255, (unsigned char)((1.0f - t) * 255), alpha };
    } else if (value < 0.75f) {
        // Green to Yellow
        float t = (value - 0.5f) / 0.25f;
        result = (creColor){ (unsigned char)(t * 255), 255, 0, alpha };
    } else {
        // Yellow to Red
        float t = (value - 0.75f) / 0.25f;
        result = (creColor){ 255, (unsigned char)((1.0f - t) * 255), 0, alpha };
    }
    return result;
}

/**
 * @brief Get velocity-based color (blue=slow, green=medium, red=fast).
 */
static creColor GetVelocityColor(float speed, unsigned char alpha) {
    float normalized = speed / MAX_VELOCITY_DISPLAY;
    return GetHeatmapColor(normalized, alpha);
}

// ============================================================================
// Forward Declarations
// ============================================================================

// World-space overlay renderers
static void RenderSpatialHashHeatmap(EntityRegistry* reg);
static void RenderEntityStateOverlay(EntityRegistry* reg);
static void RenderVelocityField(EntityRegistry* reg);
static void RenderCollisionLayers(EntityRegistry* reg);

// Screen-space legend/HUD renderers
static void RenderModeIndicator(void);
static void RenderLegendSpatialHash(void);
static void RenderLegendEntityState(void);
static void RenderLegendVelocity(void);
static void RenderLegendLayers(void);

// ============================================================================
// Public API Implementation
// ============================================================================

void DebugSystem_Init(void) {
    s_currentMode = DEBUG_MODE_OFF;
    s_debugEnabled = false;
    s_statsHudEnabled = true;
    Log(LOG_LVL_INFO, "Debug System Initialized - Press F1 to toggle, F2-F5 for modes");
}

void DebugSystem_HandleInput(EntityRegistry* reg) {
    assert(reg && "reg is NULL");

    // -------------------------------------------------------------------------
    // Mode Toggle Controls
    // -------------------------------------------------------------------------
    if (IsKeyPressed(KEY_F1)) {
        s_debugEnabled = !s_debugEnabled;
        if (s_debugEnabled) {
            if (s_currentMode == DEBUG_MODE_OFF) {
                s_currentMode = DEBUG_MODE_SPATIAL_HASH;
            }
        } else {
            s_currentMode = DEBUG_MODE_OFF;
        }
        Log(LOG_LVL_INFO, "Debug Overlay: %s", s_debugEnabled ? "ON" : "OFF");
    }
    
    // Direct mode selection (F2-F5)
    if (IsKeyPressed(KEY_F2)) { s_currentMode = DEBUG_MODE_SPATIAL_HASH; s_debugEnabled = true; }
    if (IsKeyPressed(KEY_F3)) { s_currentMode = DEBUG_MODE_ENTITY_STATE; s_debugEnabled = true; }
    if (IsKeyPressed(KEY_F4)) { s_currentMode = DEBUG_MODE_VELOCITY_FIELD; s_debugEnabled = true; }
    if (IsKeyPressed(KEY_F5)) { s_currentMode = DEBUG_MODE_COLLISION_LAYERS; s_debugEnabled = true; }
    
    // Cycle modes (F8)
    if (IsKeyPressed(KEY_F8)) {
        s_currentMode = (DebugVisualizationMode)((s_currentMode + 1) % DEBUG_MODE_COUNT);
        if (s_currentMode == DEBUG_MODE_OFF) s_currentMode = DEBUG_MODE_SPATIAL_HASH;
        s_debugEnabled = true;
    }
    
    // Stats HUD toggle (TAB)
    if (IsKeyPressed(KEY_TAB)) {
        s_statsHudEnabled = !s_statsHudEnabled;
    }
}

uint32_t DebugSystem_GetActiveCount(EntityRegistry* reg) {
    assert(reg && "reg is NULL");
    return reg->active_count;
}

/**
 * @brief Render world-space debug visualizations.
 * Call this INSIDE BeginWorldMode/EndWorldMode.
 */
void DebugSystem_RenderWorldSpace(EntityRegistry* reg) {
    assert(reg && "reg is NULL");
    
    // Skip visualization if disabled
    if (!s_debugEnabled || s_currentMode == DEBUG_MODE_OFF) {
        return;
    }
    
    // Dispatch to appropriate world-space visualization
    switch (s_currentMode) {
        case DEBUG_MODE_SPATIAL_HASH:
            RenderSpatialHashHeatmap(reg);
            break;
        case DEBUG_MODE_ENTITY_STATE:
            RenderEntityStateOverlay(reg);
            break;
        case DEBUG_MODE_VELOCITY_FIELD:
            RenderVelocityField(reg);
            break;
        case DEBUG_MODE_COLLISION_LAYERS:
            RenderCollisionLayers(reg);
            break;
        default:
            break;
    }
    
}

/**
 * @brief Main entry point - now just calls world-space overlays.
 * For backward compatibility. Call INSIDE world mode.
 */
void DebugSystem_RenderPhysicsInsight(EntityRegistry* reg) {
    // This renders world-space overlays
    DebugSystem_RenderWorldSpace(reg);
}

/**
 * @brief Render screen-space HUD elements.
 * Call this AFTER EndWorldMode.
 */
void DebugSystem_RenderScreenSpace(EntityRegistry* reg) {
    assert(reg && "reg is NULL");
    
    // Always render stats HUD if enabled
    if (s_statsHudEnabled) {
        DebugSystem_RenderStatsHUD(reg);
    }
    
    // Render mode indicator and legend if debug is enabled
    if (s_debugEnabled && s_currentMode != DEBUG_MODE_OFF) {
        RenderModeIndicator();
        
        // Render appropriate legend for current mode
        switch (s_currentMode) {
            case DEBUG_MODE_SPATIAL_HASH:
                RenderLegendSpatialHash();
                break;
            case DEBUG_MODE_ENTITY_STATE:
                RenderLegendEntityState();
                break;
            case DEBUG_MODE_VELOCITY_FIELD:
                RenderLegendVelocity();
                break;
            case DEBUG_MODE_COLLISION_LAYERS:
                RenderLegendLayers();
                break;
            default:
                break;
        }
    }
}

void DebugSystem_RenderStatsHUD(EntityRegistry* reg) {
    assert(reg && "reg is NULL");
    
    // Calculate frame time
    double currentTime = GetTime();
    double frameTime = (currentTime - s_lastFrameTime) * 1000.0;
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
    for (uint32_t i = 0; i <= bound; i++) {
        uint64_t flags = reg->state_flags[i];
        uint64_t comps = reg->component_masks[i];
        
        if (!(flags & FLAG_ACTIVE)) continue;
        activeCount++;
        
        if (flags & FLAG_SLEEPING) sleepingCount++;
        if (flags & FLAG_STATIC) staticCount++;
        if (flags & FLAG_CULLED) culledCount++;
        if (comps & COMP_PHYSICS) physicsCount++;
        
        // Awake = active AND not sleeping AND not culled
        if (!(flags & (FLAG_SLEEPING | FLAG_CULLED))) {
            awakeCount++;
        }
    }
    
    // Draw HUD background
    int hudX = 10;
    int hudY = 10;
    int hudWidth = 280;
    int hudHeight = 200;
    
    DrawRectangle(hudX, hudY, hudWidth, hudHeight, (Color){20, 20, 30, 220});
    DrawRectangleLines(hudX, hudY, hudWidth, hudHeight, (Color){80, 80, 100, 255});
    
    // Title
    DrawText("PHYSICS INSIGHT", hudX + 10, hudY + 8, 16, (Color){100, 200, 255, 255});
    DrawLine(hudX + 5, hudY + 28, hudX + hudWidth - 5, hudY + 28, (Color){60, 60, 80, 255});
    
    // Stats rows
    int rowY = hudY + 35;
    int rowSpacing = 18;
    
    char buffer[128];
    
    // Frame timing
    creColor fpsColor = (s_avgFrameTime < 16.67) ? (creColor){0, 228, 48, 255}/*GREEN*/ : (s_avgFrameTime < 33.33) ? (creColor){253, 249, 0, 255}/*YELLOW*/: (creColor){230, 41, 55, 255};/*RED*/
    snprintf(buffer, sizeof(buffer), "Frame: %.2f ms (%.0f FPS)", s_avgFrameTime, 1000.0 / s_avgFrameTime);
    DrawText(buffer, hudX + 10, rowY, 14, (Color){fpsColor.r, fpsColor.g, fpsColor.b, fpsColor.a});
    rowY += rowSpacing;
    
    // Entity counts
    snprintf(buffer, sizeof(buffer), "Entities: %u / %u", activeCount, MAX_ENTITIES);
    DrawText(buffer, hudX + 10, rowY, 14, WHITE);
    rowY += rowSpacing;
    
    snprintf(buffer, sizeof(buffer), "Physics:  %u", physicsCount);
    DrawText(buffer, hudX + 10, rowY, 14, (Color){150, 200, 255, 255});
    rowY += rowSpacing;
    
    // State breakdown with color coding
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
    rowY += rowSpacing;
    
    // Mode indicator
    DrawLine(hudX + 5, rowY - 2, hudX + hudWidth - 5, rowY - 2, (Color){60, 60, 80, 255});
    snprintf(buffer, sizeof(buffer), "Mode: %s", s_modeNames[s_currentMode]);
    DrawText(buffer, hudX + 10, rowY + 3, 12, (Color){180, 180, 200, 255});
}

DebugVisualizationMode DebugSystem_GetMode(void) {
    return s_currentMode;
}

void DebugSystem_SetMode(DebugVisualizationMode mode) {
    s_currentMode = mode;
    s_debugEnabled = (mode != DEBUG_MODE_OFF);
}

bool DebugSystem_IsEnabled(void) {
    return s_debugEnabled;
}

void DebugSystem_Draw(void) {
    // Legacy placeholder - use DebugSystem_RenderPhysicsInsight instead
}

// ============================================================================
// Mouse Hover Entity Inspection
// ============================================================================

void DebugSystem_RenderMouseHover(EntityRegistry* reg) {
    assert(reg && "reg is NULL");
    
    // Get mouse position in screen space (convert from Raylib)
    Vector2 raylibMouseScreen = GetMousePosition();
    
    // Convert screen space (window) to virtual viewport space
    ViewportSize vp = Viewport_Get();
    float screenW = (float)GetScreenWidth();
    float screenH = (float)GetScreenHeight();
    float scaleX = (screenW > 0.0f) ? (vp.width / screenW) : 1.0f;
    float scaleY = (screenH > 0.0f) ? (vp.height / screenH) : 1.0f;
    creVec2 mouseScreen = {raylibMouseScreen.x * scaleX, raylibMouseScreen.y * scaleY};
    
    // Convert to world space
    Camera2D cam = cameraSystem_GetInternal();
    creVec2 mouseWorld = cameraUtils_ScreenToWorld(mouseScreen, cam);
    
    const uint32_t bound = reg->max_used_bound;
    int32_t hoveredEntity = -1;
    
    // Find entity under mouse cursor (check collision shapes)
    for (uint32_t i = 0; i <= bound; i++) {
        uint64_t flags = reg->state_flags[i];
        uint64_t comps = reg->component_masks[i];
        if (!(flags & FLAG_ACTIVE)) continue;
        
        float px = reg->pos_x[i];
        float py = reg->pos_y[i];
        float w = reg->size_w[i];
        float h = reg->size_h[i];
        const float pivotX = reg->pivot_x[i];
        const float pivotY = reg->pivot_y[i];
        const float drawX = px - (w * pivotX);
        const float drawY = py - (h * pivotY);
        
        // Skip invalid sizes
        if (w <= 0 || h <= 0) continue;
        
        if (comps & COMP_COLLISION_Circle) {
            // Circle uses center position based on pivot-corrected draw origin
            float radius = w * 0.5f;
            float centerX = drawX + radius;
            float centerY = drawY + radius;
            float dx = mouseWorld.x - centerX;
            float dy = mouseWorld.y - centerY;
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                hoveredEntity = (int32_t)i;
                break;
            }
        } else {
            // AABB uses pivot-corrected top-left position
            if (mouseWorld.x >= drawX && mouseWorld.x <= drawX + w &&
                mouseWorld.y >= drawY && mouseWorld.y <= drawY + h) {
                hoveredEntity = (int32_t)i;
                break;
            }
        }
    }
    
    // No entity under mouse
    if (hoveredEntity < 0) return;
    
    uint32_t id = (uint32_t)hoveredEntity;
    float px = reg->pos_x[id];
    float py = reg->pos_y[id];
    float vx = reg->vel_x[id];
    float vy = reg->vel_y[id];
    uint64_t flags = reg->state_flags[id];
    
    // Decode flags into readable strings
    char flagsStr[256] = "";
    int flagsLen = 0;
    
    if (flags & FLAG_ACTIVE)       flagsLen += snprintf(flagsStr + flagsLen, sizeof(flagsStr) - flagsLen, "ACTIVE ");
    if (flags & FLAG_VISIBLE)      flagsLen += snprintf(flagsStr + flagsLen, sizeof(flagsStr) - flagsLen, "VISIBLE ");
    if (flags & FLAG_STATIC)       flagsLen += snprintf(flagsStr + flagsLen, sizeof(flagsStr) - flagsLen, "STATIC ");
    if (flags & FLAG_SLEEPING)     flagsLen += snprintf(flagsStr + flagsLen, sizeof(flagsStr) - flagsLen, "SLEEPING ");
    if (flags & FLAG_CULLED)       flagsLen += snprintf(flagsStr + flagsLen, sizeof(flagsStr) - flagsLen, "CULLED ");
    if (flags & FLAG_ALWAYS_AWAKE) flagsLen += snprintf(flagsStr + flagsLen, sizeof(flagsStr) - flagsLen, "ALWAYS_AWAKE ");
    
    // Remove trailing space
    if (flagsLen > 0 && flagsStr[flagsLen - 1] == ' ') {
        flagsStr[flagsLen - 1] = '\0';
    }
    
    // Tooltip dimensions
    int tooltipWidth = 240;
    int tooltipHeight = 100;
    int padding = 8;
    int lineHeight = 14;
    
    // Position tooltip near mouse (offset to avoid cursor overlap)
    int tooltipX = (int)mouseScreen.x + 15;
    int tooltipY = (int)mouseScreen.y + 15;
    
    // Keep tooltip on screen
    if (tooltipX + tooltipWidth > (int)vp.width) {
        tooltipX = (int)mouseScreen.x - tooltipWidth - 5;
    }
    if (tooltipY + tooltipHeight > (int)vp.height) {
        tooltipY = (int)mouseScreen.y - tooltipHeight - 5;
    }
    
    // Draw tooltip background
    DrawRectangle(tooltipX, tooltipY, tooltipWidth, tooltipHeight, (Color){20, 20, 35, 230});
    DrawRectangleLines(tooltipX, tooltipY, tooltipWidth, tooltipHeight, (Color){100, 180, 255, 255});
    
    // Draw tooltip content
    int textX = tooltipX + padding;
    int textY = tooltipY + padding;
    
    // Entity ID (header)
    char idStr[32];
    snprintf(idStr, sizeof(idStr), "Entity ID: %u", id);
    DrawText(idStr, textX, textY, 14, (Color){100, 200, 255, 255});
    textY += lineHeight + 4;
    
    // Position
    char posStr[64];
    snprintf(posStr, sizeof(posStr), "Pos: (%.1f, %.1f)", px, py);
    DrawText(posStr, textX, textY, 12, WHITE);
    textY += lineHeight;
    
    // Velocity
    char velStr[64];
    float speed = sqrtf(vx * vx + vy * vy);
    snprintf(velStr, sizeof(velStr), "Vel: (%.1f, %.1f) [%.1f]", vx, vy, speed);
    DrawText(velStr, textX, textY, 12, (Color){200, 255, 200, 255});
    textY += lineHeight;
    
    // Flags (may wrap if too long)
    DrawText("Flags:", textX, textY, 10, (Color){180, 180, 180, 255});
    textY += 12;
    
    // Wrap flags text if needed
    if (flagsLen > 30) {
        // Split into two lines
        char flagsLine1[128], flagsLine2[128];
        strncpy(flagsLine1, flagsStr, 30);
        flagsLine1[30] = '\0';
        strncpy(flagsLine2, flagsStr + 30, sizeof(flagsLine2) - 1);
        flagsLine2[sizeof(flagsLine2) - 1] = '\0';
        
        DrawText(flagsLine1, textX, textY, 10, YELLOW);
        textY += 12;
        DrawText(flagsLine2, textX, textY, 10, YELLOW);
    } else {
        DrawText(flagsStr[0] ? flagsStr : "(none)", textX, textY, 10, YELLOW);
    }
}

// ============================================================================
// Mode Indicator Overlay
// ============================================================================

static void RenderModeIndicator(void) {
    ViewportSize vp = Viewport_Get();
    int vpWidth = (int)vp.width;
    int vpHeight = (int)vp.height;
    
    const char* modeName = s_modeNames[s_currentMode];
    int textWidth = MeasureText(modeName, 20);
    
    // Draw mode name at top center
    int x = (vpWidth - textWidth) / 2;
    int y = 10;
    
    DrawRectangle(x - 10, y - 5, textWidth + 20, 30, (Color){20, 20, 30, 200});
    DrawRectangleLines(x - 10, y - 5, textWidth + 20, 30, (Color){100, 150, 255, 255});
    DrawText(modeName, x, y, 20, (Color){100, 200, 255, 255});
    
    // Controls hint at bottom
    const char* hint = "F1: Toggle | F2-F5: Modes | F8: Cycle | TAB: Stats";
    int hintWidth = MeasureText(hint, 12);
    DrawText(hint, (vpWidth - hintWidth) / 2, vpHeight - 25, 12, (Color){150, 150, 150, 200});
}

// ============================================================================
// Mode 1: Spatial Hash Heatmap
// ============================================================================

static void RenderSpatialHashHeatmap(EntityRegistry* reg) {
    const uint32_t bound = reg->max_used_bound;
    
    // Count entities per visible grid cell using a simple approach:
    // We'll sample the world in a grid and count entities in each cell
    
    ViewportSize v = Viewport_Get();
    Camera2D cam = cameraSystem_GetInternal();
    creVec2 camTarget = {cam.target.x, cam.target.y};
    
    // Calculate visible area
    float visWidth = v.width / cam.zoom;
    float visHeight = v.height / cam.zoom;
    float startX = camTarget.x - visWidth / 2;
    float startY = camTarget.y - visHeight / 2;
    
    // Grid resolution for visualization (larger cells for performance)
    int cellSize = VIS_GRID_SIZE * 2;  // Double the physics grid size for vis
    int cellsX = (int)(visWidth / cellSize) + 2;
    int cellsY = (int)(visHeight / cellSize) + 2;
    
    // Static buffer for cell counts (avoid allocation)
    static uint16_t cellCounts[128][128];
    static uint16_t maxCount = 0;
    
    // Limit grid size
    if (cellsX > 128) cellsX = 128;
    if (cellsY > 128) cellsY = 128;
    
    // Clear counts
    memset(cellCounts, 0, sizeof(cellCounts));
    maxCount = 1;  // Avoid division by zero
    
    // Count entities in each cell
    for (uint32_t i = 0; i <= bound; i++) {
        uint64_t flags = reg->state_flags[i];
        if (!(flags & FLAG_ACTIVE)) continue;
        
        float px = reg->pos_x[i];
        float py = reg->pos_y[i];
        
        int cellX = (int)((px - startX) / cellSize);
        int cellY = (int)((py - startY) / cellSize);
        
        if (cellX >= 0 && cellX < cellsX && cellY >= 0 && cellY < cellsY) {
            cellCounts[cellY][cellX]++;
            if (cellCounts[cellY][cellX] > maxCount) {
                maxCount = cellCounts[cellY][cellX];
            }
        }
    }
    
    // Draw heatmap cells
    for (int cy = 0; cy < cellsY; cy++) {
        for (int cx = 0; cx < cellsX; cx++) {
            if (cellCounts[cy][cx] == 0) continue;
            
            float density = (float)cellCounts[cy][cx] / (float)maxCount;
            creColor cellColor = GetHeatmapColor(density, HEATMAP_CELL_ALPHA);
            
            int screenX = (int)(startX + cx * cellSize);
            int screenY = (int)(startY + cy * cellSize);
            
            DrawRectangle(screenX, screenY, cellSize, cellSize, (Color){cellColor.r, cellColor.g, cellColor.b, cellColor.a});
            
            // Draw count text for high-density cells
            if (cellCounts[cy][cx] >= 5) {
                char countStr[8];
                snprintf(countStr, sizeof(countStr), "%d", cellCounts[cy][cx]);
                DrawText(countStr, screenX + 4, screenY + 4, 10, WHITE);
            }
        }
    }
    
    // Draw grid lines (subtle)
    creColor gridColor = {100, 100, 120, 50};
    for (int cx = 0; cx <= cellsX; cx++) {
        int screenX = (int)(startX + cx * cellSize);
        DrawLine(screenX, (int)startY, screenX, (int)(startY + cellsY * cellSize), (Color){gridColor.r, gridColor.g, gridColor.b, gridColor.a});
    }
    for (int cy = 0; cy <= cellsY; cy++) {
        int screenY = (int)(startY + cy * cellSize);
        DrawLine((int)startX, screenY, (int)(startX + cellsX * cellSize), screenY, (Color){gridColor.r, gridColor.g, gridColor.b, gridColor.a});
    }
    
    // Cache max count for legend (drawn in screen space)
    s_heatmapMaxCount = maxCount;
}

// ============================================================================
// Mode 2: Entity State Overlay
// ============================================================================

static void RenderEntityStateOverlay(EntityRegistry* reg) {
    const uint32_t bound = reg->max_used_bound;
    
    // Define state colors
    const creColor colorActive   = creGREEN;   // Green - Awake
    const creColor colorSleeping = creYELLOW;   // Yellow - Sleeping
    const creColor colorStatic   = creBLUE;   // Blue - Static
    const creColor colorCulled   = creRED;    // Red - Culled
    const creColor colorNaN      = { 255, 0, 0, 255 };      // Red - Data corruption
    const creColor colorOrphan   = creORANGE;    // Orange - Orphan entity
    
    // Orphan boundary threshold
    const float ORPHAN_THRESHOLD = 10000.0f;
    
    // Get viewport for orphan indicators drawn at screen edges
    ViewportSize vp = Viewport_Get();
    Camera2D cam = cameraSystem_GetInternal();
    
    // Draw entities as colored dots
    for (uint32_t i = 0; i <= bound; i++) {
        uint64_t flags = reg->state_flags[i];
        if (!(flags & FLAG_ACTIVE)) continue;
        
        float px = reg->pos_x[i];
        float py = reg->pos_y[i];
        // Pivot = {0,0}
        
        // =========================================================================
        // DATA CORRUPTION DETECTION: NaN / Inf check
        // =========================================================================
        if (isnan(px) || isnan(py) || isinf(px) || isinf(py)) {
            // Draw massive red box at camera center to alert developer
            creVec2 camPos = {cam.target.x, cam.target.y};
            int boxSize = 100;
            DrawRectangle((int)(camPos.x - boxSize/2), (int)(camPos.y - boxSize/2), 
                         boxSize, boxSize, (Color){255, 0, 0, 100});
            DrawRectangleLines((int)(camPos.x - boxSize/2), (int)(camPos.y - boxSize/2), 
                              boxSize, boxSize, R_COL(colorNaN));
            
            // Draw error text
            char nanStr[64];
            snprintf(nanStr, sizeof(nanStr), "NaN ERROR [ID:%u]", i);
            DrawText(nanStr, (int)(camPos.x - 60), (int)(camPos.y - 10), 16, R_COL(colorNaN));
            continue;  // Skip further processing for corrupted entity
        }
        
        // =========================================================================
        // ORPHAN DETECTION: Entity too far from origin
        // =========================================================================
        if (px < -ORPHAN_THRESHOLD || px > ORPHAN_THRESHOLD || 
            py < -ORPHAN_THRESHOLD || py > ORPHAN_THRESHOLD) {
            // Calculate direction to orphan from camera
            creVec2 camPos = {cam.target.x, cam.target.y};
            float dx = px - camPos.x;
            float dy = py - camPos.y;
            float dist = sqrtf(dx * dx + dy * dy);
            
            if (dist > 1.0f) {
                // Normalize direction
                float nx = dx / dist;
                float ny = dy / dist;
                
                // Draw arrow at edge of visible area pointing toward orphan
                float edgeX = camPos.x + nx * (vp.width / cam.zoom / 2.5f);
                float edgeY = camPos.y + ny * (vp.height / cam.zoom / 2.5f);
                
                // Draw indicator arrow
                DrawCircle((int)edgeX, (int)edgeY, 8, R_COL(colorOrphan));
                DrawLine((int)edgeX, (int)edgeY, 
                        (int)(edgeX + nx * 20), (int)(edgeY + ny * 20), R_COL(colorOrphan));
                
                // Draw orphan label
                char orphanStr[64];
                snprintf(orphanStr, sizeof(orphanStr), "ORPHAN [ID:%u] @%.0f,%.0f", i, px, py);
                DrawText(orphanStr, (int)(edgeX - 50), (int)(edgeY - 25), 10, R_COL(colorOrphan));
            }
            continue;  // Don't draw normal overlay for orphans (they're off-screen anyway)
        }
        
        creColor dotColor;
        int radius = ENTITY_DOT_RADIUS;
        
        if (flags & FLAG_CULLED) {
            dotColor = colorCulled;
            radius = 2;
        } else if (flags & FLAG_STATIC) {
            dotColor = colorStatic;
            radius = 4;
        } else if (flags & FLAG_SLEEPING) {
            dotColor = colorSleeping;
        } else {
            dotColor = colorActive;
        }
        
        DrawCircle((int)px, (int)py, (float)radius, R_COL(dotColor));
        
        // Draw direction indicator for awake entities
        if (!(flags & (FLAG_SLEEPING | FLAG_STATIC | FLAG_CULLED))) {
            float vx = reg->vel_x[i];
            float vy = reg->vel_y[i];
            float speed = sqrtf(vx * vx + vy * vy);
            if (speed > 1.0f) {
                float nx = vx / speed;
                float ny = vy / speed;
                DrawLine((int)px, (int)py, 
                        (int)(px + nx * 8), (int)(py + ny * 8), 
                        (Color){255, 255, 255, 150});
            }
        }
    }
    // Legend drawn in screen-space (RenderLegendEntityState)
}

// ============================================================================
// Mode 3: Velocity Field
// ============================================================================

static void RenderVelocityField(EntityRegistry* reg) {
    const uint32_t bound = reg->max_used_bound;
    
    float maxSpeed = 0.0f;
    float totalSpeed = 0.0f;
    uint32_t movingCount = 0;
    
    // First pass: find max speed and draw vectors
    for (uint32_t i = 0; i <= bound; i++) {
        uint64_t flags = reg->state_flags[i];
        if (!(flags & FLAG_ACTIVE)) continue;
        if (flags & FLAG_STATIC) continue;
        
        float px = reg->pos_x[i];
        float py = reg->pos_y[i];
        float vx = reg->vel_x[i];
        float vy = reg->vel_y[i];
        
        float speed = sqrtf(vx * vx + vy * vy);
        if (speed > maxSpeed) maxSpeed = speed;
        totalSpeed += speed;
        if (speed > 0.1f) movingCount++;
        
        // Skip very slow entities
        if (speed < 1.0f) continue;
        
        // Calculate vector endpoint (clamped length)
        float displayLen = speed * VELOCITY_SCALE;
        if (displayLen > MAX_VELOCITY_DISPLAY * VELOCITY_SCALE) {
            displayLen = MAX_VELOCITY_DISPLAY * VELOCITY_SCALE;
        }
        
        float nx = vx / speed;
        float ny = vy / speed;
        float endX = px + nx * displayLen;
        float endY = py + ny * displayLen;
        
        // Color based on speed
        creColor vecColor = GetVelocityColor(speed, 200);
        
        // Draw velocity vector
        DrawLineEx((Vector2){px, py}, (Vector2){endX, endY}, 2.0f, (Color){vecColor.r, vecColor.g, vecColor.b, vecColor.a});
        
        // Draw arrowhead
        float arrowSize = 4.0f;
        float perpX = -ny * arrowSize;
        float perpY = nx * arrowSize;
        DrawTriangle(
            (Vector2){endX, endY},
            (Vector2){endX - nx * arrowSize * 2 + perpX, endY - ny * arrowSize * 2 + perpY},
            (Vector2){endX - nx * arrowSize * 2 - perpX, endY - ny * arrowSize * 2 - perpY},
            (Color){vecColor.r, vecColor.g, vecColor.b, vecColor.a}
        );
        
        // Draw speed text for fast entities
        if (speed > 50.0f) {
            char speedStr[16];
            snprintf(speedStr, sizeof(speedStr), "%.0f", speed);
            DrawText(speedStr, (int)px + 5, (int)py - 15, 10, (Color){vecColor.r, vecColor.g, vecColor.b, vecColor.a});
        }
    }
    
    // Cache stats for legend (drawn in screen space)
    s_velocityMaxSpeed = maxSpeed;
    s_velocityAvgSpeed = movingCount > 0 ? totalSpeed / movingCount : 0.0f;
    s_velocityMovingCount = movingCount;
}

// ============================================================================
// Mode 4: Collision Layers
// ============================================================================

static void RenderCollisionLayers(EntityRegistry* reg) {
    const uint32_t bound = reg->max_used_bound;
    
    // Count entities per layer
    static uint32_t layerCounts[8];
    memset(layerCounts, 0, sizeof(layerCounts));
    
    // Draw entities colored by layer
    for (uint32_t i = 0; i <= bound; i++) {
        uint64_t flags = reg->state_flags[i];
        if (!(flags & FLAG_ACTIVE)) continue;
        if (!(flags & FLAG_VISIBLE)) continue;
        if (flags & FLAG_CULLED) continue;

        uint64_t comps = reg->component_masks[i];
        bool hasAabb = (comps & COMP_COLLISION_AABB) != 0;
        bool hasCircle = (comps & COMP_COLLISION_Circle) != 0;
        if (!hasAabb && !hasCircle) continue;
        
        float px = reg->pos_x[i];
        float py = reg->pos_y[i];
        float w = reg->size_w[i];
        float h = reg->size_h[i];
        const float pivotX = reg->pivot_x[i];
        const float pivotY = reg->pivot_y[i];

        const float drawX = px - (w * pivotX);
        const float drawY = py - (h * pivotY);
        
        // Extract layer (bits 48-55)
        uint32_t layer = (uint32_t)GET_LAYER(flags);
        
        // Find first set bit (primary layer)
        int layerIndex = 0;
        uint32_t tempLayer = layer;
        while (tempLayer > 1 && layerIndex < 7) {
            tempLayer >>= 1;
            layerIndex++;
        }
        
        if (layerIndex < 8) layerCounts[layerIndex]++;
        
        creColor layerColor = s_layerColors[layerIndex % 8];
        
        // Draw entity bounds with layer color
        if (hasAabb) {
            DrawRectangleLines((int)(drawX - 1), (int)(drawY - 1), (int)w + 2, (int)h + 2, R_COL(layerColor));
        } else if (hasCircle) {
            float radius = w * 0.5f;
            if (radius > 0.0f) {
                DrawCircleLines((int)(drawX + radius), (int)(drawY + radius), radius, R_COL(layerColor));
            }
        }
        
        // Draw small layer indicator
        if (hasCircle) {
            const float radius = w * 0.5f;
            DrawCircle((int)(drawX + radius), (int)(drawY + radius), 3, (Color){layerColor.r, layerColor.g, layerColor.b, layerColor.a});
        } else {
            DrawCircle((int)drawX, (int)drawY, 3, (Color){layerColor.r, layerColor.g, layerColor.b, layerColor.a});
        }
    }
    
    // Cache layer counts for legend (drawn in screen space)
    for (int i = 0; i < 8; i++) {
        s_layerCounts[i] = layerCounts[i];
    }
}    

// ============================================================================
// SCREEN-SPACE LEGEND RENDERING FUNCTIONS
// These render HUD legends using cached data from world-space rendering
// ============================================================================

static void RenderLegendSpatialHash(void) {
    ViewportSize vp = Viewport_Get();
    int legendX = (int)vp.width - 180;
    int legendY = 50;
    
    DrawRectangle(legendX - 5, legendY - 5, 175, 80, (Color){20, 20, 30, 200});
    DrawText("Spatial Hash Heatmap", legendX, legendY, 14, WHITE);
    
    // Gradient bar
    for (int i = 0; i < 100; i++) {
        float t = i / 99.0f;
        creColor c = GetHeatmapColor(t, 255);
        DrawRectangle(legendX + i, legendY + 25, 1, 15, (Color){c.r, c.g, c.b, c.a});
    }
    DrawText("Empty", legendX, legendY + 45, 10, (Color){100, 200, 255, 255});
    DrawText("Dense", legendX + 65, legendY + 45, 10, (Color){255, 100, 100, 255});
    
    char statsStr[32];
    snprintf(statsStr, sizeof(statsStr), "Peak: %u entities/cell", s_heatmapMaxCount);
    DrawText(statsStr, legendX, legendY + 60, 12, YELLOW);
}

static void RenderLegendEntityState(void) {
    ViewportSize vp = Viewport_Get();
    int legendX = (int)vp.width - 160;
    int legendY = 50;
    
    DrawRectangle(legendX - 5, legendY - 5, 155, 100, (Color){20, 20, 30, 200});
    DrawText("Entity States", legendX, legendY, 14, WHITE);
    
    // State colors legend
    const char* stateNames[] = {"Normal", "Hurt", "Attacking", "Dead"};
    creColor stateColors[] = {{0, 228, 48, 255}, {255, 161, 0, 255}, {230, 41, 55, 255}, {100, 100, 100, 255}};
    
    for (int i = 0; i < 4; i++) {
        int rowY = legendY + 22 + i * 18;
        DrawCircle(legendX + 8, rowY + 6, 6, (Color){stateColors[i].r, stateColors[i].g, stateColors[i].b, stateColors[i].a});
        DrawText(stateNames[i], legendX + 22, rowY, 12, (Color){stateColors[i].r, stateColors[i].g, stateColors[i].b, stateColors[i].a});
    }
}

static void RenderLegendVelocity(void) {
    ViewportSize vp = Viewport_Get();
    int legendX = (int)vp.width - 180;
    int legendY = 50;
    
    DrawRectangle(legendX - 5, legendY - 5, 175, 95, (Color){20, 20, 30, 200});
    DrawText("Velocity Field", legendX, legendY, 14, WHITE);
    
    // Speed gradient
    for (int i = 0; i < 100; i++) {
        float t = i / 99.0f;
        creColor c = GetHeatmapColor(t, 255);
        DrawRectangle(legendX + i, legendY + 25, 1, 12, (Color){c.r, c.g, c.b, c.a});
    }
    DrawText("Slow", legendX, legendY + 40, 10, (Color){100, 200, 255, 255});
    DrawText("Fast", legendX + 70, legendY + 40, 10, (Color){255, 100, 100, 255});
    
    char statsStr[48];
    snprintf(statsStr, sizeof(statsStr), "Max: %.1f u/s", s_velocityMaxSpeed);
    DrawText(statsStr, legendX, legendY + 55, 11, YELLOW);
    snprintf(statsStr, sizeof(statsStr), "Avg: %.1f u/s", s_velocityAvgSpeed);
    DrawText(statsStr, legendX, legendY + 70, 11, (Color){200, 200, 200, 255});
    snprintf(statsStr, sizeof(statsStr), "Moving: %u", s_velocityMovingCount);
    DrawText(statsStr, legendX + 85, legendY + 70, 11, GREEN);
}

static void RenderLegendLayers(void) {
    ViewportSize vp = Viewport_Get();
    int legendX = (int)vp.width - 160;
    int legendY = 50;
    int legendHeight = 140;
    
    DrawRectangle(legendX - 5, legendY - 5, 155, legendHeight, (Color){20, 20, 30, 200});
    DrawText("Collision Layers", legendX, legendY, 14, (Color){255, 255, 255, 255});
    
    const char* layerNames[] = {"L_PLAYER", "L_ENEMY", "L_BULLET", "L_WORLD", "L_PICKUP", "L_TRIGGER", "L_6", "L_7"};
    
    int rowY = legendY + 22;
    for (int i = 0; i < 8; i++) {
        if (s_layerCounts[i] == 0) continue;
        
        creColor lc = s_layerColors[i];
        DrawCircle(legendX + 8, rowY + 6, 5, (Color){lc.r, lc.g, lc.b, lc.a});
        
        char layerStr[32];
        snprintf(layerStr, sizeof(layerStr), "%s: %u", layerNames[i], s_layerCounts[i]);
        DrawText(layerStr, legendX + 20, rowY, 11, (Color){lc.r, lc.g, lc.b, lc.a});
        
        rowY += 14;
        if (rowY > legendY + legendHeight - 20) break;
    }
}    
