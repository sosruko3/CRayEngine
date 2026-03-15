#include "game_prototypes.h"
#include "assets/atlas/atlas_data.h"
#include "engine/core/cre_types.h"
#include "engine/ecs/cre_entityManager.h"
#include "engine/ecs/cre_entityRegistry.h"
#include "engine/systems/physics/cre_physicsSystem.h"
#include "entity_types.h"

Entity g_playerPrototype = (Entity){.id = 0, .generation = 0};
Entity g_zombiePrototype = (Entity){.id = 1, .generation = 0};

void Prototypes_Init(EntityRegistry *reg) {
  // Zombie prototype
  uint64_t zombieCompMask = COMP_SPRITE | COMP_PHYSICS | COMP_COLLISION_AABB;
  uint64_t zombieFlags =
      FLAG_VISIBLE | SET_LAYER(L_ENEMY) | SET_MASK(L_PLAYER | L_ENEMY);

  g_zombiePrototype = EntityManager_Create(
      reg, TYPE_ENEMY, (creVec2){110.0f, 180.0f}, zombieCompMask, zombieFlags);

  if (ENTITY_IS_VALID(g_zombiePrototype)) {
    const uint32_t zombieid = g_zombiePrototype.id;
    reg->render_layer[zombieid] = RENDER_LAYER_ENEMY;
    reg->batch_ids[zombieid] = RENDER_BATCH_ENEMY;
    reg->sprite_ids[zombieid] = SPR_ENEMY_IDLE;
    reg->material_id[zombieid] = MAT_DEFAULT;
    reg->drag[zombieid] = 2.0f;
    reg->inv_mass[zombieid] = 1.0f; // Fix this later on.
  }

  // Player prototype
  uint64_t playerCompMask = COMP_SPRITE | COMP_PHYSICS | COMP_COLLISION_AABB;

  uint64_t playerFlags = FLAG_VISIBLE | FLAG_ALWAYS_AWAKE |
                         SET_LAYER(L_PLAYER) | SET_MASK(L_ENEMY | L_BULLET);

  g_playerPrototype = EntityManager_Create(
      reg, TYPE_PLAYER, (creVec2){100, 200}, playerCompMask, playerFlags);
  if (ENTITY_IS_VALID(g_playerPrototype)) {
    const uint32_t playerid = g_playerPrototype.id;
    reg->render_layer[playerid] = RENDER_LAYER_ENEMY;
    reg->batch_ids[playerid] = RENDER_BATCH_ENEMY;
    reg->sprite_ids[playerid] = SPR_FISH_BLUE;
    reg->material_id[playerid] = MAT_PLAYER;
    reg->drag[playerid] = 0.2f;
    reg->inv_mass[playerid] = 1.0f;
  }
}
