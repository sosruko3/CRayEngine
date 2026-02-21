#ifndef CRE_ENTITYAPI_H
#define CRE_ENTITYAPI_H

#include <stdint.h>
typedef struct Entity Entity;
typedef struct CommandBus CommandBus;
typedef struct EntityRegistry EntityRegistry;
typedef struct creVec2 creVec2;

Entity entityAPI_ReserveID(EntityRegistry* reg);

void entityAPI_Clone(EntityRegistry* reg,
                     CommandBus* restrict bus,
					 Entity              dst,
					 Entity              prototype,
					 creVec2             position);

void entityAPI_Spawn(CommandBus* restrict bus, Entity entity, uint16_t type, creVec2 position);

void entityAPI_SpawnUntracked(CommandBus* restrict bus, uint16_t type, creVec2 position);

void entityAPI_Destroy(CommandBus* restrict bus, Entity entity);

void entityAPI_SetType(CommandBus* restrict bus,Entity entity,uint16_t type);

void entityAPI_SetFlags(void); // fill this one

void entityAPI_SetPivot(CommandBus* restrict bus,Entity entity,creVec2 pivot);
// not sure about creVec2 pivot, maybe float pivot_x and float pivot_y?



#endif
