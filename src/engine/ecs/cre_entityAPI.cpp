#include "cre_entityAPI.h"
#include "cre_entityManager.h"
#include "engine/core/cre_commandBus.h"
#include "engine/core/cre_logger.h"
#include <assert.h>

Entity entityAPI_ReserveSlot(EntityRegistry &reg) {
  return EntityManager_ReserveSlot(reg);
}

Entity entityAPI_Spawn(EntityRegistry &reg, CommandBus &bus, Entity prototype,
                       creVec2 position) {
  Entity reserved = entityAPI_ReserveSlot(reg);
  if (!ENTITY_IS_VALID(reserved))
    return ENTITY_INVALID;

  Command cmd = {.type = CMD_ENTITY_SPAWN,
                 .entity = reserved,
                 .entityClone = CommandPayloadEntityClone{
                     .prototype = prototype,
                     .position = position,
                 }};

  if (!CommandBus_Push(bus, cmd)) {
    EntityManager_ReturnReservedSlot(reg, reserved);
    Log(LogLevel::Error, "entityAPI_Spawn: CommandBus is full! Slot returned.");
    return ENTITY_INVALID;
  }

  return reserved;
}

Entity entityAPI_SpawnUntracked(EntityRegistry &reg, CommandBus &bus,
                                Entity prototype, creVec2 position) {
  Entity reserved = entityAPI_ReserveSlot(reg);
  if (!ENTITY_IS_VALID(reserved))
    return ENTITY_INVALID;

  Command cmd = {.type = CMD_ENTITY_SPAWN_UNTRACKED,
                 .entity = reserved,
                 .entityClone = CommandPayloadEntityClone{
                     .prototype = prototype,
                     .position = position,
                 }};

  if (!CommandBus_Push(bus, cmd)) {
    EntityManager_ReturnReservedSlot(reg, reserved);
    Log(LogLevel::Error,
        "entityAPI_SpawnUntracked: CommandBus is full! Slot returned.");
    return ENTITY_INVALID;
  }

  return reserved;
}

void entityAPI_Destroy(CommandBus &bus, Entity entity) {
  Command cmd = {
      .type = CMD_ENTITY_DESTROY,
      .entity = entity,
      .u64 = {},
  };

  if (!CommandBus_Push(bus, cmd)) {
    Log(LogLevel::Warning, "entityAPI_Destroy: CommandBus is full!");
  }
}

void entityAPI_AddFlags(CommandBus &bus, Entity entity, uint64_t flags) {
  Command cmd = {
      .type = CMD_ENTITY_ADD_FLAGS,
      .entity = entity,
      .u64 = CommandPayloadU64{.value = flags},
  };

  if (!CommandBus_Push(bus, cmd)) {
    Log(LogLevel::Warning, "entityAPI_SetFlags: CommandBus is full!");
  }
}

void entityAPI_RemoveFlags(CommandBus &bus, Entity entity, uint64_t flags) {
  Command cmd = {
      .type = CMD_ENTITY_REMOVE_FLAGS,
      .entity = entity,
      .u64 = CommandPayloadU64{.value = flags},
  };

  if (!CommandBus_Push(bus, cmd)) {
    Log(LogLevel::Warning, "entityAPI_ClearFlags: CommandBus is full!");
  }
}

void entityAPI_SetType(CommandBus &bus, Entity entity, uint16_t type) {
  Command cmd = {
      .type = CMD_ENTITY_SET_TYPE,
      .entity = entity,
      .u16 = CommandPayloadU16{.value = type},
  };

  if (!CommandBus_Push(bus, cmd)) {
    Log(LogLevel::Warning, "entityAPI_SetType: CommandBus is full!");
  }
}

void entityAPI_SetPivot(CommandBus &bus, Entity entity, creVec2 pivot) {
  Command cmd = {
      .type = CMD_ENTITY_SET_PIVOT,
      .entity = entity,
      .vec2 = CommandPayloadVec2{.value = pivot},
  };

  if (!CommandBus_Push(bus, cmd)) {
    Log(LogLevel::Warning, "entityAPI_SetPivot: CommandBus is full!");
  }
}

void entityAPI_AddComponent(CommandBus &bus, Entity entity,
                            uint64_t component_mask) {
  Command cmd = {
      .type = CMD_ENTITY_ADD_COMPONENT,
      .entity = entity,
      .u64 = CommandPayloadU64{.value = component_mask},
  };

  if (!CommandBus_Push(bus, cmd)) {
    Log(LogLevel::Warning, "entityAPI_AddComponent: CommandBus is full!");
  }
}

void entityAPI_RemoveComponent(CommandBus &bus, Entity entity,
                               uint64_t component_mask) {
  Command cmd = {
      .type = CMD_ENTITY_REMOVE_COMPONENT,
      .entity = entity,
      .u64 = CommandPayloadU64{.value = component_mask},
  };

  if (!CommandBus_Push(bus, cmd)) {
    Log(LogLevel::Warning, "entityAPI_RemoveComponent: CommandBus is full!");
  }
}

void entityAPI_Clone(EntityRegistry &reg, CommandBus &bus, Entity dst,
                     Entity prototype, creVec2 position) {

  Command cmd = {.type = CMD_ENTITY_CLONE,
                 .entity = dst,
                 .entityClone = CommandPayloadEntityClone{
                     .prototype = prototype,
                     .position = position,
                 }};

  if (!CommandBus_Push(bus, cmd)) {
    EntityManager_ReturnReservedSlot(reg, dst);
    Log(LogLevel::Warning, "entityAPI_Clone: CommandBus is full! Slot returned.");
  }
}
