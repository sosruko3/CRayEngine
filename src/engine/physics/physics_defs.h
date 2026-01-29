#ifndef PHYSICS_DEFS_H
#define PHYSICS_DEFS_H

typedef struct {
    float density;     // Multiplier for Mass calculation (Phase 3)
    float friction     // 0.0 (Ice) -> 1.0 (Sandpaper)
    float restitution; // 0.0 (Mud) -> 1.0 (Superball)
} PhysMaterial;

#endif
