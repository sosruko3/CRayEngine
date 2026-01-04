#include "input.h"
#include "raylib.h"

static int keyBindings[ACTION_COUNT];

void Input_Init(void) {
    // Default bindings -- Load from file later

    // Movement
    keyBindings[ACTION_UP]    = KEY_W;
    keyBindings[ACTION_DOWN]  = KEY_S;
    keyBindings[ACTION_LEFT]  = KEY_A;
    keyBindings[ACTION_RIGHT] = KEY_D;

    // Menu
    keyBindings[ACTION_CONFIRM] = KEY_ENTER;
    keyBindings[ACTION_BACK]    = KEY_ESCAPE;
    keyBindings[ACTION_PAUSE]   = KEY_TAB;

    // Actions
    keyBindings[ACTION_PRIMARY]   = KEY_SPACE;
    keyBindings[ACTION_SECONDARY] = KEY_LEFT_SHIFT;

}

bool Input_IsPressed(GameAction action) {
    int key = keyBindings[action];
    return IsKeyPressed(key);
}

bool Input_IsDown(GameAction action) {
    int key =keyBindings[action];
    return IsKeyDown(key);
}

void Input_Remap(GameAction action, int key) {
    keyBindings[action] = key;
}









