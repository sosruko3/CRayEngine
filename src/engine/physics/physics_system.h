#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

void PhysicsSystem_Init(void);

/* Main Physics Pipeline:
1. Move Entities
2. Build Spatial Hash
3. Solve Collisions
*/
void PhysicsSystem_Update(float dt);
#endif