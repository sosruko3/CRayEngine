#ifndef PHYSICS_DEFS_H
#define PHYSICS_DEFS_H

#define PHYS_MAX_MATERIALS 256

typedef struct {
    float density;     // Multiplier for Mass calculation (Phase 3)
    float friction;    // 0.0 (Ice) -> 1.0 (Sandpaper)
    float restitution; // 0.0 (Mud) -> 1.0 (Superball)
} PhysMaterial;

typedef enum {
    MAT_DEFAULT = 0,
    MAT_STATIC,
    MAT_BOUNCY,
    MAT_ICE,
    MAT_COUNT
} MaterialID;


#endif
