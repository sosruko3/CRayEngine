#include "cre_physicsAPI.h"
#include "cre_physics_defs.h"
#include "engine/core/cre_commandBus.h"
#include "engine/core/cre_logger.h"
#include "engine/ecs/cre_entityRegistry.h"
#include <assert.h>
#include <cstdint>

void physicsAPI_DefineBody(CommandBus &bus, Entity entity, uint8_t mat_id,
                           float drag, bool is_static) {

  Command cmd = {.type = CMD_PHYS_DEFINE,
                 .entity = entity,
                 .physDef = {
                     .material_id = mat_id,
                     .flags = is_static
                                  ? static_cast<uint8_t>(CMD_PHYS_FLAG_STATIC)
                                  : static_cast<uint8_t>(0),
                     .drag = drag,
                 }};

  if (!CommandBus_Push(bus, cmd)) {
    Log(LogLevel::Warning, "physicsAPI_DefineBody: CommandBus is full!");
  }
}

void physicsAPI_TeleportBody(CommandBus &bus, Entity entity, float x, float y) {

  Command cmd = {.type = CMD_PHYS_TELEPORT,
                 .entity = entity,
                 .vec2 = {.value = creVec2{.x = x, .y = y}}};

  if (!CommandBus_Push(bus, cmd)) {
    Log(LogLevel::Warning, "physicsAPI_TeleportBody: CommandBus is full!");
  }
}

void physicsAPI_ApplyImpulse(CommandBus &bus, Entity entity, float jx,
                             float jy) {

  Command cmd = {.type = CMD_PHYS_APPLY_IMPULSE,
                 .entity = entity,
                 .vec2 = {.value = creVec2{.x = jx, .y = jy}}};

  if (!CommandBus_Push(bus, cmd)) {
    Log(LogLevel::Warning, "physicsAPI_ApplyImpulse: CommandBus is full!");
  }
}

void physicsAPI_SetVelocity(CommandBus &bus, Entity entity, float vx,
                            float vy) {

  Command cmd = {.type = CMD_PHYS_SET_VELOCITY,
                 .entity = entity,
                 .vec2 = {.value = creVec2{.x = vx, .y = vy}}};

  if (!CommandBus_Push(bus, cmd)) {
    Log(LogLevel::Warning, "physicsAPI_SetVelocity: CommandBus is full!");
  }
}

void physicsAPI_SetDrag(CommandBus &bus, Entity entity, float drag) {

  Command cmd = {
      .type = CMD_PHYS_SET_DRAG, .entity = entity, .f32 = {.value = drag}};

  if (!CommandBus_Push(bus, cmd)) {
    Log(LogLevel::Warning, "physicsAPI_SetDrag: CommandBus is full!");
  }
}

void physicsAPI_SetGravityScale(CommandBus &bus, Entity entity, float scale) {

  Command cmd = {.type = CMD_PHYS_SET_GRAVITY_SCALE,
                 .entity = entity,
                 .f32 = {.value = scale}};

  if (!CommandBus_Push(bus, cmd)) {
    Log(LogLevel::Warning, "physicsAPI_SetGravityScale: CommandBus is full!");
  }
}

void physicsAPI_SetMaterial(CommandBus &bus, Entity entity, uint8_t mat_id) {

  Command cmd = {
      .type = CMD_PHYS_SET_MATERIAL, .entity = entity, .u8 = {.value = mat_id}};

  if (!CommandBus_Push(bus, cmd)) {
    Log(LogLevel::Warning, "physicsAPI_SetMaterial: CommandBus is full!");
  }
}

void physicsAPI_SetGlobalGravity(CommandBus &bus, float x, float y) {

  Command cmd = {.type = CMD_PHYS_SET_GRAVITY,
                 .entity = ENTITY_INVALID,
                 .vec2 = {.value = creVec2{.x = x, .y = y}}};

  CommandBus_Push(bus, cmd);
}

void physicsAPI_LoadStaticGeometry(CommandBus &bus) {

  Command cmd = {
      .type = CMD_PHYS_LOAD_STATIC,
      .entity = ENTITY_INVALID,
      .u16 = {},
  };

  CommandBus_Push(bus, cmd);
}

void physicsAPI_ResetWorld(CommandBus &bus) {

  Command cmd = {
      .type = CMD_PHYS_RESET,
      .entity = ENTITY_INVALID,
      .u16 = {},
  };

  CommandBus_Push(bus, cmd);
}

creVec2 physicsAPI_GetVelocity(const EntityRegistry &reg, Entity entity) {
  if (!EntityRegistry_IsAlive(reg, entity)) {
    return creVec2{0.0f, 0.0f};
  }

  const uint32_t id = entity.id;
  return reg.vel[id];
}

bool physicsAPI_IsSleeping(const EntityRegistry &reg, Entity entity) {
  if (!EntityRegistry_IsAlive(reg, entity)) {
    return false;
  }

  return (reg.state_flags[entity.id] & FLAG_SLEEPING) != 0;
}

uint8_t physicsAPI_GetMaterial(const EntityRegistry &reg, Entity entity) {
  if (!EntityRegistry_IsAlive(reg, entity)) {
    return MAT_DEFAULT;
  }

  return reg.material_id[entity.id];
}
