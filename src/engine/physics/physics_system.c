/**
 * @file physics_system.c
 * @brief 4-Phase Physics Pipeline Implementation
 * 
 * Data-Oriented physics system operating on EntityRegistry SoA arrays.
 * Implements: Commands → Integration → Broad Phase → Solver pipeline.
 * 
 * Design Principles:
 *   - SIMD-safe loops (separate operations, no aliasing)
 *   - Cache-friendly iteration via max_used_bound
 *   - Configurable sub-stepping and solver iterations
 *   - Material-based collision response (friction, restitution)
 */
#include "engine/core/entity_registry.h"
#include "engine/core/command_bus.h"
#include "physics_system.h"
#include "spatial_hash.h"
#include <math.h>
#include "engine/core/logger.h"
#include "engine/core/config.h"
#include "raymath.h"

// ============================================================================
// Compile-Time Validation
// ============================================================================

#if PHYS_SUB_STEPS <= 0
    #error "PHYS_SUB_STEPS must be >= 1"
#endif

#if PHYS_SOLVER_ITERATIONS <= 0
    #error "PHYS_SOLVER_ITERATIONS must be >= 1"
#endif

// MSVC needs __restrict instead of restrict (SIMD part)
#if defined(_MSC_VER)
    #define restrict __restrict
#endif

// ============================================================================
// NaN/Inf Detection (Debug Builds)
// ============================================================================

#define FLOAT_IS_VALID(f) (isfinite(f))
#define SANITIZE_FLOAT(f) (FLOAT_IS_VALID(f) ? (f) : 0.0f)


// ============================================================================
// Static Configuration
// ============================================================================

/** Global gravity vector (modifiable via PhysicsSystem_SetGravity) */
static float g_gravity_x = PHYS_GRAVITY_DEF_X;
static float g_gravity_y = PHYS_GRAVITY_DEF_Y;

/** 
 * Material lookup table.
 * Index by reg->material_id[entity] to get friction/restitution.
 */
static PhysMaterial g_materials[PHYS_MAX_MATERIALS] = {
    [MAT_DEFAULT] = { .density = 1.0f,  .friction = 0.5f,  .restitution = 0.0f },
    [MAT_STATIC]  = { .density = 0.0f,  .friction = 0.5f,  .restitution = 0.0f },
    [MAT_BOUNCY]  = { .density = 1.0f,  .friction = 0.3f,  .restitution = 0.9f },
    [MAT_ICE]     = { .density = 1.0f,  .friction = 0.05f, .restitution = 0.0f },
    // Remaining slots zero-initialized
};

// ============================================================================
// Forward Declarations (Internal Helpers)
// ============================================================================

static void ConfigureBody(EntityRegistry* reg, uint32_t id, uint8_t mat_id, float drag, bool is_static);
static void Phase1_Integration(EntityRegistry* reg, float dt);
static void Phase2_BroadPhase(EntityRegistry* reg);
static void Phase3_DetectContacts(EntityRegistry* reg);
static void Phase3_ResolveContacts(EntityRegistry* reg);

// Collision detection helpers (return true if colliding)
static bool CheckCircle_Circle(
    float ax, float ay, float radiusA,
    float bx, float by, float radiusB,
    float* out_overlap, Vector2* out_normal);

static bool CheckAABB_AABB(
    float ax, float ay, float aw, float ah,
    float bx, float by, float bw, float bh,
    float* out_overlap, Vector2* out_normal);

static bool CheckCircle_AABB(
    float cx, float cy, float radius,
    float bx, float by, float bw, float bh,
    float* out_overlap, Vector2* out_normal);

// Collision response
static void ResolveCollision(
    EntityRegistry* reg,
    uint32_t idA, uint32_t idB,
    float overlap, Vector2 normal);

// ============================================================================
// #3 Optimization: Contact Stream (Cache-Friendly Collision Resolution)
// ============================================================================
// Instead of detecting + resolving interleaved (cache misses on each pair),
// collect all contact pairs first, then resolve in a sequential pass.

#define MAX_CONTACTS 65536

/**
 * @brief Contact pair for deferred resolution.
 */
typedef struct {
    uint32_t idA;
    uint32_t idB;
    float overlap;
    float normalX;
    float normalY;
} ContactPair;

static ContactPair contactStream[MAX_CONTACTS];
static int contactCount = 0;

// ============================================================================
// Public API Implementation
// ============================================================================

void PhysicsSystem_Init(void) {
    // Clear all spatial hash layers
    SpatialHash_ClearAll();
    
    // Reset gravity to defaults
    g_gravity_x = PHYS_GRAVITY_DEF_X;
    g_gravity_y = PHYS_GRAVITY_DEF_Y;
    
    Log(LOG_LVL_INFO, "Physics System Initialized (SubSteps=%d, SolverIters=%d)",
        PHYS_SUB_STEPS, PHYS_SOLVER_ITERATIONS);
}

void PhysicsSystem_Update(EntityRegistry* reg, CommandBus* bus, float dt) {
    if (!reg) return;
    
    // Clamp dt to valid range
    if (dt <= 0.0f) return;  // Skip zero/negative time
    if (dt > 0.05f) dt = 0.05f;  // Prevent spiral of death
    
    // -------------------------------------------------------------------------
    // Phase 0: Process Commands (runs once per frame, outside sub-step loop)
    // -------------------------------------------------------------------------
    if (bus) {
        PhysicsSystem_ProcessCommands(reg, bus);
    }
    
    const float subDt = dt / (float)PHYS_SUB_STEPS;
    
    // -------------------------------------------------------------------------
    // Sub-Step Loop
    // -------------------------------------------------------------------------
    for (int step = 0; step < PHYS_SUB_STEPS; step++) {
        // Phase 1: Integration (gravity, drag, movement)
        Phase1_Integration(reg, subDt);
        
        // Phase 2: Broad Phase (rebuild dynamic spatial hash)
        Phase2_BroadPhase(reg);
        
        // Phase 3: Collision Solver (split for cache efficiency)
        // Multiple iterations for stability
        for (int iter = 0; iter < PHYS_SOLVER_ITERATIONS; iter++) {
            Phase3_DetectContacts(reg);   // Build contact stream
            Phase3_ResolveContacts(reg);  // Resolve all contacts sequentially
        }
    }
}

void PhysicsSystem_ProcessCommands(EntityRegistry* reg, const CommandBus* bus) {
    if (!reg || !bus) return;
    
    CommandIterator iter = CommandBus_GetIterator(bus);
    const Command* cmd;
    
    while (CommandBus_Next(bus, &iter, &cmd)) {
        const uint32_t id = cmd->entityID;
        
        switch (cmd->type) {
            case CMD_PHYS_DEFINE: {
                // Configure physics body from command payload
                if (id >= MAX_ENTITIES) break;
                
                const bool isStatic = (cmd->physDef.flags & CMD_PHYS_FLAG_STATIC) != 0;
                ConfigureBody(reg, id, cmd->physDef.material_id, cmd->physDef.drag, isStatic);
                break;
            }
            
            case CMD_PHYS_LOAD_STATIC: {
                // Scan registry and populate static spatial hash
                PhysicsSystem_LoadStaticGeometry(reg);
                break;
            }
            
            case CMD_PHYS_RESET: {
                // Clear all spatial hashes (e.g., on scene unload)
                SpatialHash_ClearAll();
                break;
            }
            
            default:
                // Not a physics command, ignore
                break;
        }
    }
}

void PhysicsSystem_LoadStaticGeometry(const EntityRegistry* reg) {
    if (!reg) return;
    
    uint32_t staticCount = 0;
    const uint32_t bound = reg->max_used_bound;
    
    for (uint32_t i = 0; i <= bound; i++) {
        const uint64_t flags = reg->state_flags[i];
        const uint64_t comps = reg->component_masks[i];
        
        // Filter: Active + Physics component + Static (or infinite mass)
        if (!(flags & FLAG_ACTIVE)) continue;
        if (!(comps & COMP_PHYSICS)) continue;
        if (!(flags & FLAG_STATIC) && reg->inv_mass[i] > 0.0f) continue;
        
        // Add to static layer
        SpatialHash_AddStatic(
            i,
            (int)reg->pos_x[i],
            (int)reg->pos_y[i],
            (int)reg->size_w[i],
            (int)reg->size_h[i]
        );
        staticCount++;
    }
    
    Log(LOG_LVL_INFO, "Loaded %u static bodies into spatial hash", staticCount);
}

bool PhysicsSystem_SetMaterial(uint8_t id, PhysMaterial mat) {
    if (id >= PHYS_MAX_MATERIALS) {
        Log(LOG_LVL_WARNING, "SetMaterial: ID %u exceeds max (%u)", id, PHYS_MAX_MATERIALS);
        return false;
    }
    g_materials[id] = mat;
    return true;
}

void PhysicsSystem_SetGravity(float x, float y) {
    g_gravity_x = x;
    g_gravity_y = y;
}

// ============================================================================
// Internal Helper: Configure Body from Material
// ============================================================================

/**
 * @brief Configure physics body properties based on material.
 * 
 * Calculates inverse mass from shape area and material density.
 * Sets FLAG_STATIC and inv_mass=0 for static bodies.
 * 
 * @param reg       Entity registry
 * @param id        Entity index
 * @param mat_id    Material ID for lookup
 * @param drag      Air resistance coefficient
 * @param is_static Whether body is immovable
 */
static void ConfigureBody(EntityRegistry* reg, uint32_t id, uint8_t mat_id, float drag, bool is_static) {
    // Bounds check: prevent OOB array access
    if (id >= MAX_ENTITIES) {
        Log(LOG_LVL_ERROR, "ConfigureBody: ID %u exceeds MAX_ENTITIES (%u)", id, MAX_ENTITIES);
        return;
    }
    
    // Safety: clamp material ID
    if (mat_id >= PHYS_MAX_MATERIALS) mat_id = MAT_DEFAULT;
    
    const float density = g_materials[mat_id].density;
    const uint64_t comps = reg->component_masks[id];
    
    // Calculate area based on collision shape
    float area = 0.0f;
    if (comps & COMP_COLLISION_AABB) {
        // Box: width * height
        area = reg->size_w[id] * reg->size_h[id];
    } else if (comps & COMP_COLLISION_Circle) {
        // Circle: π * r² (size_w holds diameter)
        const float r = reg->size_w[id] * 0.5f;
        area = 3.14159265f * r * r;
    }
    
    // Configure mass and static flag
    if (is_static) {
        reg->inv_mass[id] = 0.0f;            // Infinite mass
        reg->state_flags[id] |= FLAG_STATIC;
    } else {
        const float mass = area * density;
        reg->inv_mass[id] = (mass > 0.0001f) ? (1.0f / mass) : 0.0f;
        reg->state_flags[id] &= ~FLAG_STATIC;
    }
    
    // Store material ID and drag for solver
    reg->material_id[id] = mat_id;
    reg->drag[id] = drag;
    reg->gravity_scale[id] = 1.0f;
}

// ============================================================================
// Phase 1: Integration (SIMD-Safe Structure)
// ============================================================================

/**
 * @brief Apply gravity, drag, and integrate position using Semi-Implicit Euler.
 * 
 * Structured for auto-vectorization:
 *   - Separate loops for each operation
 *   - No cross-iteration dependencies
 *   - Uses restrict-style access patterns
 * 
 * @param reg  Entity registry
 * @param dt   Sub-step delta time
 */
static void Phase1_Integration(EntityRegistry* reg, float dt) {
    const uint32_t bound = reg->max_used_bound;
    const float sleepThresholdSq = PHYS_SLEEP_EPSILON * PHYS_SLEEP_EPSILON;
    
    // Direct array access with restrict for SIMD optimization
    float* restrict const pos_x = reg->pos_x;
    float* restrict const pos_y = reg->pos_y;
    float* restrict const vel_x = reg->vel_x;
    float* restrict const vel_y = reg->vel_y;
    const float* restrict const gravity_scale = reg->gravity_scale;
    const float* restrict const drag = reg->drag;
    const uint64_t* restrict const comps = reg->component_masks;
    uint64_t* restrict const flags = reg->state_flags;
    
    // Required component mask for physics processing
    const uint64_t reqComps = COMP_PHYSICS;
    // Skip mask: static or sleeping entities don't integrate
    const uint64_t skipFlags = FLAG_STATIC | FLAG_SLEEPING;
    
    // -------------------------------------------------------------------------
    // Loop 1: Apply Gravity
    // -------------------------------------------------------------------------
    for (uint32_t i = 0; i <= bound; i++) {
        if (!(flags[i] & FLAG_ACTIVE)) continue;
        if (!(comps[i] & reqComps)) continue;
        if (flags[i] & skipFlags) continue;
        
        // Gravity 
        vel_x[i] += g_gravity_x * gravity_scale[i] * dt;
        vel_y[i] += g_gravity_y * gravity_scale[i] * dt;
        
        // Loop 2: Apply Drag (clamped to prevent sign flip)
        const float dragFactor = 1.0f - (drag[i] * dt);
        const float clampedDrag = (dragFactor > 0.0f) ? dragFactor : 0.0f;
        vel_x[i] *= clampedDrag;
        vel_y[i] *= clampedDrag;
        
        // Loop 3: Integrate Position (Semi-Implicit Euler)
        pos_x[i] += vel_x[i] * dt;
        pos_y[i] += vel_y[i] * dt;
        
        // Loop 4: Branchless Sleep Check
        const float speedSq = vel_x[i] * vel_x[i] + vel_y[i] * vel_y[i];
        bool isSlow = (speedSq < sleepThresholdSq);
        bool canSleep = !(flags[i] & FLAG_ALWAYS_AWAKE);
        if (isSlow && canSleep) {
            flags[i] |= FLAG_SLEEPING;
        }
    }
}

// ============================================================================
// Phase 2: Broad Phase (Spatial Hash Population)
// ============================================================================

/**
 * @brief Rebuild dynamic spatial hash for collision detection.
 * 
 * Clears dynamic layer and re-inserts all active, non-static physics entities.
 * Static entities remain in the persistent static layer.
 * 
 * @param reg  Entity registry
 */
static void Phase2_BroadPhase(EntityRegistry* reg) {
    // Clear dynamic layer (static layer persists)
    SpatialHash_ClearDynamic();
    
    const uint32_t bound = reg->max_used_bound;
    const uint64_t reqComps = COMP_PHYSICS;
    const uint64_t skipFlags = FLAG_STATIC | FLAG_CULLED;
    
    for (uint32_t i = 0; i <= bound; i++) {
        const uint64_t flags = reg->state_flags[i];
        const uint64_t comps = reg->component_masks[i];
        
        if (!(flags & FLAG_ACTIVE)) continue;
        if (!(comps & reqComps)) continue;
        if (flags & skipFlags) continue;
        
        SpatialHash_AddDynamic(
            i,
            (int)reg->pos_x[i],
            (int)reg->pos_y[i],
            (int)reg->size_w[i],
            (int)reg->size_h[i]
        );
    }
}

// ============================================================================
// Phase 3a: Contact Detection (Collect Pairs)
// ============================================================================

/**
 * @brief Detect all collision pairs and store in contact stream.
 * 
 * For each dynamic entity:
 *   1. Query spatial hash for nearby entities
 *   2. Dispatch to appropriate collision check (Circle/AABB)
 *   3. Store contact pair (no resolution yet)
 * 
 * Duplicate prevention: For dynamic-dynamic pairs, only process if idA < idB.
 * 
 * @param reg  Entity registry
 */
static void Phase3_DetectContacts(EntityRegistry* reg) {
    uint32_t neighbours[PHYS_MAX_NEIGHBOURS];
    const uint32_t bound = reg->max_used_bound;
    const uint64_t reqComps = COMP_PHYSICS;
    
    // Reset contact stream
    contactCount = 0;
    
    for (uint32_t i = 0; i <= bound; i++) {
        const uint64_t flagsA = reg->state_flags[i];
        const uint64_t compsA = reg->component_masks[i];
        
        // Filter: Active, has physics, not static, not sleeping
        if (!(flagsA & FLAG_ACTIVE)) continue;
        if (!(compsA & reqComps)) continue;
        if (flagsA & FLAG_STATIC) continue;
        if (flagsA & FLAG_SLEEPING) continue;
        
        // Query spatial hash for potential colliders
        const int count = SpatialHash_Query(
            (int)reg->pos_x[i],
            (int)reg->pos_y[i],
            (int)reg->size_w[i],
            (int)reg->size_h[i],
            neighbours,
            PHYS_MAX_NEIGHBOURS
        );
        
        // Check against each potential collider
        for (int k = 0; k < count; k++) {
            const uint32_t j = neighbours[k];
            
            // Validate entity ID from spatial hash
            if (j >= MAX_ENTITIES) {
                Log(LOG_LVL_ERROR, "SpatialHash returned invalid ID: %u", j);
                continue;
            }
            
            // Skip self
            if (j == i) continue;
            
            const uint64_t flagsB = reg->state_flags[j];
            const uint64_t compsB = reg->component_masks[j];
            
            // Must be active
            if (!(flagsB & FLAG_ACTIVE)) continue;
            
            // Duplicate prevention: For dynamic-dynamic, only solve if i < j
            const bool bIsStatic = (flagsB & FLAG_STATIC) || (reg->inv_mass[j] <= 0.0f);
            if (!bIsStatic) {
                // Both dynamic: skip if i > j, unless j is sleeping
                // (sleeping entities are filtered from outer loop, so won't process this pair)
                if (i > j && !(flagsB & FLAG_SLEEPING)) continue;
            }
            
            // ----------------------------------------------------------------
            // Collision Layer/Mask Check
            // ----------------------------------------------------------------
            const uint32_t layerA = (uint32_t)GET_LAYER(flagsA);
            const uint32_t maskA  = (uint32_t)GET_MASK(flagsA);
            const uint32_t layerB = (uint32_t)GET_LAYER(flagsB);
            const uint32_t maskB  = (uint32_t)GET_MASK(flagsB);
            
            // Check if they should collide (bidirectional mask check)
            if (!((maskA & layerB) || (maskB & layerA))) continue;
            
            // ----------------------------------------------------------------
            // Dispatch Collision Detection Based on Shape Types
            // ----------------------------------------------------------------
            float overlap = 0.0f;
            Vector2 normal = {0.0f, 0.0f};
            bool collided = false;
            
            const bool aIsCircle = (compsA & COMP_COLLISION_Circle) != 0;
            const bool bIsCircle = (compsB & COMP_COLLISION_Circle) != 0;
            const bool aIsAABB   = (compsA & COMP_COLLISION_AABB) != 0;
            const bool bIsAABB   = (compsB & COMP_COLLISION_AABB) != 0;
            
            if (aIsCircle && bIsCircle) {
                // Circle vs Circle
                collided = CheckCircle_Circle(
                    reg->pos_x[i], reg->pos_y[i], reg->size_w[i] * 0.5f,
                    reg->pos_x[j], reg->pos_y[j], reg->size_w[j] * 0.5f,
                    &overlap, &normal
                );
            } else if (aIsAABB && bIsAABB) {
                // AABB vs AABB
                collided = CheckAABB_AABB(
                    reg->pos_x[i], reg->pos_y[i], reg->size_w[i], reg->size_h[i],
                    reg->pos_x[j], reg->pos_y[j], reg->size_w[j], reg->size_h[j],
                    &overlap, &normal
                );
            } else if (aIsCircle && bIsAABB) {
                // Circle (A) vs AABB (B)
                collided = CheckCircle_AABB(
                    reg->pos_x[i], reg->pos_y[i], reg->size_w[i] * 0.5f,
                    reg->pos_x[j], reg->pos_y[j], reg->size_w[j], reg->size_h[j],
                    &overlap, &normal
                );
            } else if (aIsAABB && bIsCircle) {
                // AABB (A) vs Circle (B) - swap and invert normal
                collided = CheckCircle_AABB(
                    reg->pos_x[j], reg->pos_y[j], reg->size_w[j] * 0.5f,
                    reg->pos_x[i], reg->pos_y[i], reg->size_w[i], reg->size_h[i],
                    &overlap, &normal
                );
                // Invert normal since we swapped A/B
                normal.x = -normal.x;
                normal.y = -normal.y;
            }
            
            // ----------------------------------------------------------------
            // Store Contact if Collision Detected
            // ----------------------------------------------------------------
            if (collided && overlap > 0.0f) {
                if (contactCount < MAX_CONTACTS) {
                    contactStream[contactCount].idA = i;
                    contactStream[contactCount].idB = j;
                    contactStream[contactCount].overlap = overlap;
                    contactStream[contactCount].normalX = normal.x;
                    contactStream[contactCount].normalY = normal.y;
                    contactCount++;
                }
            }
        }
    }
}

// ============================================================================
// Phase 3b: Contact Resolution (Sequential Stream Iteration)
// ============================================================================

/**
 * @brief Resolve all collected contacts sequentially.
 * 
 * Iterates over contact stream and applies collision response.
 * Sequential access pattern is cache-friendly.
 * 
 * @param reg  Entity registry
 */
static void Phase3_ResolveContacts(EntityRegistry* reg) {
    for (int i = 0; i < contactCount; i++) {
        const ContactPair* c = &contactStream[i];
        Vector2 normal = { c->normalX, c->normalY };
        ResolveCollision(reg, c->idA, c->idB, c->overlap, normal);
    }
}

// ============================================================================
// Collision Detection Helpers
// ============================================================================

/**
 * @brief Check circle-circle collision.
 * 
 * @param ax, ay     Center of circle A
 * @param radiusA    Radius of circle A
 * @param bx, by     Center of circle B
 * @param radiusB    Radius of circle B
 * @param out_overlap Output: penetration depth (positive = overlapping)
 * @param out_normal  Output: normal from A to B
 * @return true if circles overlap
 */
static bool CheckCircle_Circle(
    float ax, float ay, float radiusA,
    float bx, float by, float radiusB,
    float* out_overlap, Vector2* out_normal)
{
    const float dx = bx - ax;  // Direction from A to B
    const float dy = by - ay;
    const float distSq = dx * dx + dy * dy;
    const float combinedRadius = radiusA + radiusB;
    const float combinedRadiusSq = combinedRadius * combinedRadius;
    
    if (distSq >= combinedRadiusSq) {
        return false;  // No collision
    }
    
    if (distSq < 0.0001f) {
        // Circles at same position - use arbitrary normal
        *out_normal = (Vector2){1.0f, 0.0f};
        *out_overlap = combinedRadius;
        return true;
    }
    
    const float dist = sqrtf(distSq);
    *out_overlap = combinedRadius - dist;
    *out_normal = (Vector2){dx / dist, dy / dist};  // Normalized, A to B
    return true;
}

/**
 * @brief Check AABB-AABB collision using Minimum Translation Vector.
 * 
 * Finds the axis with smallest overlap (MTV) and returns that as the
 * separation axis. Normal is axis-aligned (not diagonal).
 * 
 * @param ax, ay     Top-left corner of AABB A
 * @param aw, ah     Width and height of AABB A
 * @param bx, by     Top-left corner of AABB B
 * @param bw, bh     Width and height of AABB B
 * @param out_overlap Output: smallest penetration depth
 * @param out_normal  Output: axis-aligned normal from A to B
 * @return true if AABBs overlap
 */
static bool CheckAABB_AABB(
    float ax, float ay, float aw, float ah,
    float bx, float by, float bw, float bh,
    float* out_overlap, Vector2* out_normal)
{
    // Calculate half-sizes and centers
    const float halfAW = aw * 0.5f;
    const float halfAH = ah * 0.5f;
    const float halfBW = bw * 0.5f;
    const float halfBH = bh * 0.5f;
    
    const float centerAX = ax + halfAW;
    const float centerAY = ay + halfAH;
    const float centerBX = bx + halfBW;
    const float centerBY = by + halfBH;
    
    // Distance between centers
    const float dx = centerBX - centerAX;
    const float dy = centerBY - centerAY;
    
    // Combined half-widths
    const float overlapX = (halfAW + halfBW) - fabsf(dx);
    const float overlapY = (halfAH + halfBH) - fabsf(dy);
    
    // Check for separation
    if (overlapX <= 0.0f || overlapY <= 0.0f) {
        return false;
    }
    
    // Find MTV (Minimum Translation Vector) - smallest overlap axis
    if (overlapX < overlapY) {
        *out_overlap = overlapX;
        *out_normal = (Vector2){(dx > 0.0f) ? 1.0f : -1.0f, 0.0f};
    } else {
        *out_overlap = overlapY;
        *out_normal = (Vector2){0.0f, (dy > 0.0f) ? 1.0f : -1.0f};
    }
    
    return true;
}

/**
 * @brief Check circle-AABB collision.
 * 
 * Finds closest point on AABB to circle center, then checks distance.
 * 
 * @param cx, cy     Circle center
 * @param radius     Circle radius
 * @param bx, by     Top-left corner of AABB
 * @param bw, bh     Width and height of AABB
 * @param out_overlap Output: penetration depth
 * @param out_normal  Output: normal from circle to AABB closest point
 * @return true if overlapping
 */
static bool CheckCircle_AABB(
    float cx, float cy, float radius,
    float bx, float by, float bw, float bh,
    float* out_overlap, Vector2* out_normal)
{
    // Clamp circle center to AABB bounds to find closest point
    const float closestX = (cx < bx) ? bx : ((cx > bx + bw) ? bx + bw : cx);
    const float closestY = (cy < by) ? by : ((cy > by + bh) ? by + bh : cy);
    
    // Distance from circle center to closest point
    const float dx = closestX - cx;
    const float dy = closestY - cy;
    const float distSq = dx * dx + dy * dy;
    const float radiusSq = radius * radius;
    
    if (distSq >= radiusSq) {
        return false;  // No collision
    }
    
    if (distSq < 0.0001f) {
        // Circle center inside AABB - find escape direction
        // Use center-to-center direction
        const float aabbCenterX = bx + bw * 0.5f;
        const float aabbCenterY = by + bh * 0.5f;
        const float escapeX = cx - aabbCenterX;
        const float escapeY = cy - aabbCenterY;
        const float escapeDist = sqrtf(escapeX * escapeX + escapeY * escapeY);
        
        if (escapeDist > 0.0001f) {
            *out_normal = (Vector2){escapeX / escapeDist, escapeY / escapeDist};
        } else {
            *out_normal = (Vector2){1.0f, 0.0f};  // Arbitrary
        }
        *out_overlap = radius;  // Full radius overlap
        return true;
    }
    
    const float dist = sqrtf(distSq);
    *out_overlap = radius - dist;
    // Normal points from circle to closest point (toward AABB)
    *out_normal = (Vector2){dx / dist, dy / dist};
    return true;
}

// ============================================================================
// Collision Response
// ============================================================================

/**
 * @brief Apply collision response: wake, position correction, impulse, friction.
 * 
 * Response steps:
 *   1. Wake both entities (clear FLAG_SLEEPING)
 *   2. Position correction (Linear Projection with slop)
 *   3. Velocity impulse (Newtonian restitution)
 *   4. Friction impulse (Coulomb model)
 * 
 * @param reg     Entity registry
 * @param idA     First entity
 * @param idB     Second entity
 * @param overlap Penetration depth
 * @param normal  Collision normal (from A to B)
 */
static void ResolveCollision(
    EntityRegistry* reg,
    uint32_t idA, uint32_t idB,
    float overlap, Vector2 normal)
{
    // -------------------------------------------------------------------------
    // Step 1: Wake Both Entities
    // -------------------------------------------------------------------------
    reg->state_flags[idA] &= ~FLAG_SLEEPING;
    reg->state_flags[idB] &= ~FLAG_SLEEPING;
    
    // -------------------------------------------------------------------------
    // Get inverse masses (0 = infinite mass / static)
    // -------------------------------------------------------------------------
    const float invMassA = reg->inv_mass[idA];
    const float invMassB = reg->inv_mass[idB];
    const float invMassSum = invMassA + invMassB;
    
    // Both static = no response needed (increased epsilon to prevent explosion)
    if (invMassSum <= 0.0001f) return;
    
    // -------------------------------------------------------------------------
    // Get material properties (with defensive clamping)
    // -------------------------------------------------------------------------
    const uint8_t matA = reg->material_id[idA];
    const uint8_t matB = reg->material_id[idB];
    
    // Defensive clamping (material_id could be set externally)
    const uint8_t safeMatA = (matA < PHYS_MAX_MATERIALS) ? matA : MAT_DEFAULT;
    const uint8_t safeMatB = (matB < PHYS_MAX_MATERIALS) ? matB : MAT_DEFAULT;
    
    const float frictionA = g_materials[safeMatA].friction;
    const float frictionB = g_materials[safeMatB].friction;
    const float restitutionA = g_materials[safeMatA].restitution;
    const float restitutionB = g_materials[safeMatB].restitution;
    
    // Mix: max for restitution, multiply for friction
    const float e = (restitutionA > restitutionB) ? restitutionA : restitutionB;
    const float mu = frictionA * frictionB;
    
    // -------------------------------------------------------------------------
    // Step 2: Position Correction (Linear Projection)
    // -------------------------------------------------------------------------
    float correctionMag = ((overlap - PHYS_SLOP) > 0.0f ? (overlap - PHYS_SLOP) : 0.0f)
                          * PHYS_CORRECTION_PERCENT / invMassSum;
    
    // Cap maximum correction to prevent teleportation (M4 fix)
    const float MAX_CORRECTION = 50.0f;  // pixels per frame
    if (correctionMag > MAX_CORRECTION) {
        correctionMag = MAX_CORRECTION;
    }
    
    reg->pos_x[idA] -= normal.x * correctionMag * invMassA;
    reg->pos_y[idA] -= normal.y * correctionMag * invMassA;
    reg->pos_x[idB] += normal.x * correctionMag * invMassB;
    reg->pos_y[idB] += normal.y * correctionMag * invMassB;
    
    // -------------------------------------------------------------------------
    // Step 3: Velocity Impulse (Restitution)
    // -------------------------------------------------------------------------
    const float relVelX = reg->vel_x[idA] - reg->vel_x[idB];
    const float relVelY = reg->vel_y[idA] - reg->vel_y[idB];
    const float velAlongNormal = relVelX * normal.x + relVelY * normal.y;
    
    // Don't resolve if velocities are separating 
    if (velAlongNormal < -0.001f) return;
    
    // Impulse magnitude
    const float j = -(1.0f + e) * velAlongNormal / invMassSum;

    // Apply impulse
    reg->vel_x[idA] += j * invMassA * normal.x;
    reg->vel_y[idA] += j * invMassA * normal.y;
    reg->vel_x[idB] -= j * invMassB * normal.x;
    reg->vel_y[idB] -= j * invMassB * normal.y;
    
    // -------------------------------------------------------------------------
    // Step 4: Friction Impulse (Tangent)
    // -------------------------------------------------------------------------
    // Recalculate relative velocity after normal impulse
    const float relVelX2 = reg->vel_x[idA] - reg->vel_x[idB];
    const float relVelY2 = reg->vel_y[idA] - reg->vel_y[idB];
    
    // Tangent vector (perpendicular to normal)
    const float tangentX = relVelX2 - (relVelX2 * normal.x + relVelY2 * normal.y) * normal.x;
    const float tangentY = relVelY2 - (relVelX2 * normal.x + relVelY2 * normal.y) * normal.y;
    const float tangentLen = sqrtf(tangentX * tangentX + tangentY * tangentY);
    
    if (tangentLen < 0.0001f) return;  // No tangent velocity
    
    // Normalize tangent
    const float tangentNormX = tangentX / tangentLen;
    const float tangentNormY = tangentY / tangentLen;
    
    // Friction impulse magnitude
    float jt = -(relVelX2 * tangentNormX + relVelY2 * tangentNormY) / invMassSum;
    
    // Clamp to Coulomb friction (|jt| <= |j| * mu)
    const float maxFriction = fabsf(j) * mu;
    if (jt < -maxFriction) jt = -maxFriction;
    if (jt > maxFriction) jt = maxFriction;
    
    // Apply friction impulse
    reg->vel_x[idA] += jt * invMassA * tangentNormX;
    reg->vel_y[idA] += jt * invMassA * tangentNormY;
    reg->vel_x[idB] -= jt * invMassB * tangentNormX;
    reg->vel_y[idB] -= jt * invMassB * tangentNormY;
}
