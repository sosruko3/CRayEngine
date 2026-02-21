#include "cre_entityAPI.h"
#include "cre_entityManager.h"
#include "engine/core/cre_commandBus.h"

#include <assert.h>

Entity entityAPI_ReserveID(EntityRegistry* reg) {
	return EntityManager_ReserveSlot(reg);
}

void entityAPI_Clone(CommandBus* restrict bus,
					 Entity              dst,
					 Entity              prototype,
					 creVec2             position) {

	Command cmd = {
		.type = CMD_ENTITY_CLONE,
		.entity = dst,
		.entityClone = (CommandPayloadEntityClone){
			.prototype = prototype,
			.position = position,
		},
	};

	const bool pushed = CommandBus_Push(bus, cmd);
	assert(pushed && "entityAPI_Clone: CommandBus is full");
}
