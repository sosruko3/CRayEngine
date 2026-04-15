#include "controlSystem.h"
#include "atlas_data.h"
#include "engine/core/cre_commandBus.h"
#include "engine/core/cre_config.h"
#include "engine/ecs/cre_components.h"
#include "engine/ecs/cre_entityAPI.h"
#include "engine/ecs/cre_entityManager.h"
#include "engine/ecs/cre_entityRegistry.h"
#include "engine/platform/cre_input.h"
#include "engine/platform/cre_viewport.h"
#include "engine/systems/animation/cre_animationAPI.h"
#include "engine/systems/audio/cre_audioAPI.h"
#include "engine/systems/camera/cre_cameraAPI.h"
#include "engine/systems/camera/cre_cameraSystem.h"
#include "engine/systems/physics/cre_physicsAPI.h"
#include "engine/systems/physics/cre_physicsSystem.h"
#include "engine/systems/physics/cre_physics_defs.h"
#include "entity_types.h"
#include "game_prototypes.h"
#include "raylib.h"
#include <assert.h>
#include <math.h>

#define SLEEP_RADIUS 2500.0f
#define SLEEP_RADIUS_SQR (SLEEP_RADIUS * SLEEP_RADIUS)
#define SPAWN_COUNT 100
#define PLAYER_SPEED 400.0f
#define SCALE_FACTOR 4.0f
#define ZOOM_RATE_PER_SEC 0.60f

static Entity getActiveCameraEntity(const EntityRegistry &reg) {
  int32_t camIdx = cameraSystem_FindActive(reg);
  if (camIdx < 0 || camIdx >= static_cast<int32_t>(MAX_CAMERAS)) {
    return ENTITY_INVALID;
  }
  return reg.cameras[camIdx].ownerEntity;
}

void ControlSystem_UpdateLogic(EntityRegistry &reg, float dt,
                               creRectangle cullBounds) {
  (void)dt;
  uint32_t maxBound = reg.max_used_bound;
  float boundMinX = cullBounds.x;
  float boundMaxX = cullBounds.x + cullBounds.width;
  float boundMinY = cullBounds.y;
  float boundMaxY = cullBounds.y + cullBounds.height;
  for (uint32_t i = 0; i < maxBound; i++) {
    if (!(reg.state_flags[i] & FLAG_ACTIVE))
      continue;

    float posX = reg.pos[i].x;
    float posY = reg.pos[i].y;
    bool isOutofBounds = (posX < boundMinX || posX > boundMaxX ||
                          posY < boundMinY || posY > boundMaxY);

    switch (reg.types[i]) {
    case TYPE_PLAYER: {
      // Input Control
      float velX = 0, velY = 0;
      float speed = PLAYER_SPEED;
      if (Input_IsDown(ACTION_UP))
        velY = -speed;
      if (Input_IsDown(ACTION_DOWN))
        velY = speed;
      if (Input_IsDown(ACTION_LEFT))
        velX = -speed;
      if (Input_IsDown(ACTION_RIGHT))
        velX = speed;
      reg.vel[i] = creVec2{velX, velY};
      break;
    }
    case TYPE_PARTICLE: {
      if (isOutofBounds) {
        Entity self = {.id = i, .generation = reg.generations[i]};
        EntityManager_Destroy(reg, self);
      }
      break;
    }
    default:
      break;
    }
  }
}

void ControlSystem_ChangeZoom(EntityRegistry &reg, CommandBus &bus, float dt) {
  if (dt <= 0.0f)
    return;

  Entity camEntity = getActiveCameraEntity(reg);
  if (!EntityRegistry_IsAlive(reg, camEntity))
    return;

  int32_t camIdx = cameraSystem_FindActive(reg);
  float currentZoom = (camIdx >= 0) ? reg.cameras[camIdx].zoom : 1.0f;

  if (Input_IsDown(ACTION_PRIMARY)) {
    float zoomScale = expf(ZOOM_RATE_PER_SEC * dt);
    cameraAPI_SetZoom(bus, camEntity, currentZoom * zoomScale);
  } else if (Input_IsDown(ACTION_SECONDARY)) {
    float zoomScale = expf(-ZOOM_RATE_PER_SEC * dt);
    cameraAPI_SetZoom(bus, camEntity, currentZoom * zoomScale);
  }
}

void ControlSystem_SetCameraTarget(EntityRegistry &reg, CommandBus &bus,
                                   Entity target, Entity camEntity) {

  if (!ENTITY_IS_VALID(target))
    return;

  if (!EntityRegistry_IsAlive(reg, camEntity))
    return;

  // Target can be a reserved handle from entityAPI_Spawn and may become alive
  // later in the same frame when EntitySystem consumes commands.
  cameraAPI_SetFollowTarget(bus, camEntity, target, 10.0f, creVec2{0.0f, 0.0f});

  // Snap immediately only when target already exists in registry.
  if (EntityRegistry_IsAlive(reg, target)) {
    creVec2 pos = reg.pos[target.id];
    reg.pos[camEntity.id].x = pos.x;
    reg.pos[camEntity.id].y = pos.y;
  }
}

void ControlSystem_UpdateSleepState(EntityRegistry &reg,
                                    const CameraComponent *cam) {

  cam = cameraSystem_GetActiveComponent(reg);
  if (!cam)
    return;

  float centerX = cam->viewPosition.x;
  float centerY = cam->viewPosition.y;
  uint32_t maxBound = reg.max_used_bound;

  for (uint32_t i = 0; i < maxBound; i++) {
    // Only check active, non-player entities
    if (!(reg.state_flags[i] & FLAG_ACTIVE))
      continue;
    if (reg.types[i] == TYPE_PLAYER)
      continue;

    // Distance Check
    float dx = reg.pos[i].x - centerX;
    float dy = reg.pos[i].y - centerY;
    float distSqr = (dx * dx) + (dy * dy);

    // Set the Flag
    if (distSqr > SLEEP_RADIUS_SQR) {
      reg.state_flags[i] |= FLAG_CULLED; // Set bit
    } else {
      reg.state_flags[i] &= ~FLAG_CULLED; // Clear bit
    }
  }
}

void ControlSystem_HandleDebugSpawning(EntityRegistry &reg, CommandBus &bus) {
  if (IsKeyPressed(KEY_Z)) {
    ViewportSize v = Viewport_Get();

    /// AUDIO SFX TEST
    audioAPI_PlayOneShot(bus, AUDIO_GROUP_MASTER, AUDIO_SOURCE_TEST_SFX);
    ///
    for (int i = 0; i < SPAWN_COUNT; i++) {
      int x = GetRandomValue(static_cast<int>(-8 * v.width),
                             static_cast<int>(v.width * 8));
      int y = GetRandomValue(static_cast<int>(-8 * v.height),
                             static_cast<int>(v.height * 8));

      Entity zombie = entityAPI_Spawn(
          reg, bus, g_zombiePrototype,
          creVec2{static_cast<float>(x), static_cast<float>(y)});
      // not a beautiful solution but it works. Will be handling these casts
      // later on.
      physicsAPI_DefineBody(bus, zombie, MAT_DEFAULT, 2.0f, false);

      animAPI_Play(bus, zombie, ANIM_CHARACTER_ZOMBIE_RUN, true);
    }
  }

  if (IsKeyPressed(KEY_X)) {
    /// BGM TESTING!!!!
    const AudioID bgmID = audioAPI_AllocateSound();
    audioAPI_SoundLoad(bus, bgmID, AUDIO_SOURCE_TEST_BGM, AUDIO_GROUP_MASTER,
                       AUDIO_USAGE_STREAM);
    audioAPI_SoundSetVolume(bus, bgmID, 0.5f);
    audioAPI_SoundSetPitch(bus, bgmID, 1.0f);
    audioAPI_SoundSetPan(bus, bgmID, 0.0f);
    audioAPI_SoundPlay(bus, bgmID);
  }
}

Entity ControlSystem_SpawnPlayer(EntityRegistry &reg, CommandBus &bus) {
  Entity player =
      entityAPI_Spawn(reg, bus, g_playerPrototype, creVec2{100, 200});
  physicsAPI_DefineBody(bus, player, MAT_PLAYER, 0.2f, false);
  // animAPI_Play(bus, player, ANIM_CHARACTER_ZOMBIE_RUN, true);
  return player;
}

Entity ControlSystem_SpawnCamera(EntityRegistry &reg) {
  Entity camEntity = EntityManager_Create(
      reg, TYPE_CAMERA, creVec2{100.0f, 200.0f}, COMP_CAMERA, FLAG_ACTIVE);

  CameraComponent cam = cameraSystem_CreateDefault();
  cam.ownerEntity = camEntity;
  cam.isActive = true;
  cam.zoom = 0.4f;

  reg.cameras[reg.camera_count++] = cam;
  return camEntity;
}
