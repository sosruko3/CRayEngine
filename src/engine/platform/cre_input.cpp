#include "cre_input.h"
#include "engine/core/cre_logger.h"
#include "raylib.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Fix naming inconsistencies in this file too.
// Haven't decided what would be the prefix.
static int32_t keyBindings[ACTION_COUNT];

struct ActionMapping {
  // Do we really need pointers?
  const char *name;
  GameAction action;
};

static constexpr ActionMapping actionTable[] = {{"UP", ACTION_UP},
                                                {"DOWN", ACTION_DOWN},
                                                {"LEFT", ACTION_LEFT},
                                                {"RIGHT", ACTION_RIGHT},
                                                {"CONFIRM", ACTION_CONFIRM},
                                                {"BACK", ACTION_BACK},
                                                {"PAUSE", ACTION_PAUSE},
                                                {"PRIMARY", ACTION_PRIMARY},
                                                {"SECONDARY", ACTION_SECONDARY},
                                                {nullptr, ACTION_INVALID}};

struct KeyMapping {
  const char *name;
  int key;
};

static constexpr KeyMapping keyTable[] = {{"SPACE", KEY_SPACE},
                                          {"ENTER", KEY_ENTER},
                                          {"ESCAPE", KEY_ESCAPE},
                                          {"UP", KEY_UP},
                                          {"DOWN", KEY_DOWN},
                                          {"LEFT", KEY_LEFT},
                                          {"RIGHT", KEY_RIGHT},
                                          {"TAB", KEY_TAB},
                                          {"SHIFT", KEY_LEFT_SHIFT},
                                          {"LEFT_SHIFT", KEY_LEFT_SHIFT},
                                          {"CONTROL", KEY_LEFT_CONTROL},
                                          {"LEFT_CONTROL", KEY_LEFT_CONTROL},
                                          {nullptr, KEY_NULL}};

// Find action ID from String
GameAction GetActionFromStr(const char *str) {
  for (int i = 0; actionTable[i].name != nullptr; i++) {
    if (strcmp(str, actionTable[i].name) == 0)
      return actionTable[i].action;
  }
  return ACTION_INVALID;
}

// Find Key ID from String
int GetKeyFromStr(const char *str) {
  if (strlen(str) == 1) {
    char c = str[0];
    if (c >= 'a' && c <= 'z')
      c -= 32;
    return static_cast<int>(c);
  }
  for (int i = 0; keyTable[i].name != nullptr; i++) {
    if (strcmp(str, keyTable[i].name) == 0)
      return keyTable[i].key;
  }
  return KEY_NULL;
}

void Input_LoadConfig(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    Log(LogLevel::Warning, "Config missing: {}. Using defaults.", filename);
    return;
  }

  char line[128];
  while (fgets(line, sizeof(line), file)) {
    char *actionName = strtok(line, " =\n\r\t");
    char *keyName = strtok(nullptr, " =\n\r\t");

    // safety check
    if (!actionName || !keyName)
      continue;

    GameAction action = GetActionFromStr(actionName);
    if (action == ACTION_INVALID)
      continue;

    int key = GetKeyFromStr(keyName);
    if (key == KEY_NULL)
      continue;

    Input_Remap(action, key);
  }
  fclose(file);
  Log(LogLevel::Info, "Config loaded successfully.");
}

void Input_Init(const char *configPath) {
  // Default bindings --

  // Movement
  keyBindings[ACTION_UP] = KEY_W;
  keyBindings[ACTION_DOWN] = KEY_S;
  keyBindings[ACTION_LEFT] = KEY_A;
  keyBindings[ACTION_RIGHT] = KEY_D;

  // Menu
  keyBindings[ACTION_CONFIRM] = KEY_ENTER;
  keyBindings[ACTION_BACK] = KEY_ESCAPE;
  keyBindings[ACTION_PAUSE] = KEY_TAB;

  // Actions
  keyBindings[ACTION_PRIMARY] = KEY_SPACE;
  keyBindings[ACTION_SECONDARY] = KEY_LEFT_SHIFT;

  Input_LoadConfig(configPath);
}

bool Input_IsPressed(GameAction action) {
  int32_t key = keyBindings[action];
  return IsKeyPressed(key);
}

bool Input_IsDown(GameAction action) {
  int32_t key = keyBindings[action];
  return IsKeyDown(key);
}

void Input_Remap(GameAction action, int32_t key) { keyBindings[action] = key; }

void Input_Poll(void) {}
