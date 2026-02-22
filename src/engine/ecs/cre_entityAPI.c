#include "cre_entityAPI.h"
#include "cre_entityManager.h"
#include "engine/core/cre_commandBus.h"
#include "engine/core/cre_logger.h"

#include <assert.h>

Entity entityAPI_ReserveSlot(EntityRegistry* reg) {
	return EntityManager_ReserveSlot(reg);
}

void entityAPI_Clone(EntityRegistry* reg,
                    CommandBus* restrict bus,
					 Entity              dst,
					 Entity              prototype,
					 creVec2             position) {

	Command cmd = {
		.type = CMD_ENTITY_CLONE,
		.entity = dst,
		.entityClone = (CommandPayloadEntityClone){
			.prototype = prototype,
			.position = position,
		}
	};

    if (!CommandBus_Push(bus, cmd)) {
        EntityManager_ReturnReservedSlot(reg, dst);
        Log(LOG_LVL_ERROR, "entityAPI_Clone: CommandBus is full! Slot returned.");
    }
}
