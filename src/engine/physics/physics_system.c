#include "physics_system.h"
#include "spatial_hash.h"
#include "../core/entity_manager.h"
#include "raymath.h" // Raylib's math
#include <math.h> // for sqrtf
#include "../core/logger.h"
#include "config.h"

#define SUB_STEPS 4
#define SLEEP_EPSILON 2.0f
// configuration
// 0.5 = Smooth push, 1.0 is hard snap.
#define SEPARATION_FORCE 0.5f

static inline bool ShouldCollide(EntityData* a, EntityData* b) {
    // get the data first
    uint32_t layerA = GET_LAYER(a->flags);
    uint32_t maskA  = GET_MASK(a->flags);
    uint32_t layerB = GET_LAYER(b->flags);
    uint32_t maskB  = GET_MASK(b->flags);
    // logic part
    if (maskA & layerB) return true; // does a want to hit b
    if (maskB & layerA) return true; // does b want to hit a
    return false;
}
void PhysicsSystem_Init(void) {
    SpatialHash_Clear();
    // add conditions for logs
    Log(LOG_LVL_INFO,"Physics System Initialized.");
}
void PhysicsSystem_Update(float dt) {
    // 1. Integration
    // slowing down if fps is below 20
    if (dt > 0.05f) dt = 0.05f;
    float sleepSq = SLEEP_EPSILON * SLEEP_EPSILON;

    // Doing clear func once, for optimization.
    SpatialHash_Clear();

    // 2. Broad phase (Build the hash) This parts runs once for optimization. Build 1x/Solve 4x solution.
    for(uint32_t i = 0;i < MAX_ENTITIES;i++) {
        EntityData* e = &entityStore[i];
        if (!(e->flags & FLAG_ACTIVE)) continue;

        // Sleep check, optimization.
        float speedSq = Vector2LengthSqr(e->velocity);
        bool isSleeping = (speedSq < sleepSq && !(e->flags & FLAG_ALWAYS_AWAKE));
        if (isSleeping) e->flags |= FLAG_SLEEPING;
        else e->flags &= ~FLAG_SLEEPING;

        // move the entity
        if(!isSleeping) {
            e->position.x += e->velocity.x * dt; // changed subDt to dt for optimization
            e->position.y += e->velocity.y * dt;
        }
        SpatialHash_Add(i ,(int)e->position.x ,(int)e->position.y ,(int)e->size.x ,(int)e->size.y );
    }
    // SUB-STEP LOOP
    for(uint32_t step = 0; step < SUB_STEPS;step++) {
        // 3. Narrow phase (the solver loop)
        uint32_t neighbours[32];
        
        for (uint32_t i = 0;i < MAX_ENTITIES;i++) {
            EntityData* e = &entityStore[i];
            
            if (!(e->flags & (FLAG_ACTIVE | FLAG_SLEEPING))) continue;
        
            // calculate it
            int count = SpatialHash_Query(
                (int)e->position.x, (int)e->position.y,
                (int)e->size.x,     (int)e->size.y,
                neighbours,         32);
                // check against potential neighbours
            for (int k = 0; k < count;k++) {
                uint32_t otherID = neighbours[k]; 
                // self check
                if (otherID == i) continue; // check this again
                    
                EntityData* other = &entityStore[otherID];
                if (!(other->flags & FLAG_ACTIVE)) continue;

                bool other_isSleeping = (other->flags & FLAG_SLEEPING);
                if (otherID < i && !other_isSleeping) continue;

                if (!(ShouldCollide(e,other))) continue;
                    
                // circle collision check, this part is dirty! Clean it later on
                Vector2 diff = Vector2Subtract(e->position,other->position);
                float distSq = Vector2LengthSqr(diff);
                float radiusA = e->size.x * 0.5f;
                float radiusB = other->size.x * 0.5f;
                float combinedRadius = radiusA + radiusB;
                float combinedRadiusSq = combinedRadius * combinedRadius;
                    
                // check if circles overlap 
                if (distSq < combinedRadiusSq && distSq >  0.0001f) {
                    // Collision detected
                    float dist = sqrtf(distSq);
                    float overlap = combinedRadius - dist;
                        
                    // calculate push direction (normal)
                    Vector2 normal = Vector2Scale(diff,1.0f/dist);
                    
                    // move both entites apart
                    float pushFactor = (overlap * SEPARATION_FORCE) / SUB_STEPS;
                    Vector2 separation = Vector2Scale(normal,pushFactor);
                    e->position = Vector2Add(e->position,separation);
                    other->position = Vector2Subtract(other->position,separation);

                    if(other_isSleeping) {
                        other->flags &= ~FLAG_SLEEPING;
                    }

                    // --- IMPULSE RESOLUTION (Bounce) --- THIS IS TEMPORARY ----- Dirty code!
                    // Relative velocity
                    if (!((e->flags | other->flags) & FLAG_BOUNCY)) continue;
                    Vector2 rv = Vector2Subtract(e->velocity, other->velocity);
                    float velAlongNormal = Vector2DotProduct(rv, normal);
 
                    // Do not resolve if velocities are separating
                    if (velAlongNormal > 0) continue;
 
                    // Calculate restitution (bounciness) - could be property of entity later
                    float eRes = 0.5f;
 
                    // Calculate impulse scalar
                    float j = -(1.0f + eRes) * velAlongNormal;
                    j /= 2.0f; // 1/massA + 1/massB (Assuming mass = 1 for both)
 
                    // Apply impulse
                    Vector2 impulse = Vector2Scale(normal, j);
                    e->velocity = Vector2Add(e->velocity, impulse);
                    other->velocity = Vector2Subtract(other->velocity, impulse);
                }
            }
        }
    }     
}
