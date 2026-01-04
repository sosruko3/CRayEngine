#include "input.h"
#include "raylib.h"
#include <stdio.h>
#include <string.h>
#include "logger.h"

static int keyBindings[ACTION_COUNT];

typedef struct {
    const char* name; 
    GameAction action;
} ActionMapping;

static const ActionMapping actionTable[] = {
    { "UP",        ACTION_UP },
    { "DOWN",      ACTION_DOWN },
    { "LEFT",      ACTION_LEFT },
    { "RIGHT",     ACTION_RIGHT },
    { "CONFIRM",   ACTION_CONFIRM },
    { "BACK",      ACTION_BACK },
    { "PAUSE",     ACTION_PAUSE },
    { "PRIMARY",   ACTION_PRIMARY },
    { "SECONDARY", ACTION_SECONDARY },
    { NULL,        0 } // 
};

typedef struct {
    const char* name;
    int key;
} KeyMapping;

static const KeyMapping keyTable[] = {
    { "SPACE",    KEY_SPACE }, 
    { "ENTER",    KEY_ENTER }, 
    { "ESCAPE",   KEY_ESCAPE },
    { "UP",       KEY_UP }, 
    { "DOWN",     KEY_DOWN }, 
    { "LEFT",     KEY_LEFT }, 
    { "RIGHT",    KEY_RIGHT },
    { "TAB",      KEY_TAB }, 
    { "SHIFT",    KEY_LEFT_SHIFT }, 
    { "LEFT_SHIFT", KEY_LEFT_SHIFT },
    { "CONTROL",  KEY_LEFT_CONTROL },
    { "LEFT_CONTROL", KEY_LEFT_CONTROL },
    { NULL,       0 } 
};
 
// Find action ID from String
GameAction GetActionFromStr(const char* str) {
    for (int i = 0; actionTable[i].name != NULL;i++) {
        if (strcmp(str,actionTable[i].name) == 0) return actionTable[i].action;
    }
    return -1;
}

// Find Key ID from String
int GetKeyFromStr(const char* str) {
    if (strlen(str) == 1) {
        char c = str[0];
        if (c >= 'a' && c <= 'z') c -=32;
        return (int)c;
    }
    for (int i = 0;keyTable[i].name != NULL;i++) {
        if (strcmp(str,keyTable[i].name) == 0) return keyTable[i].key;
    }
    return 0;
}

void Input_LoadConfig(const char* filename) {
    FILE* file = fopen(filename,"r");
    if (!file) {
        Log(LOG_LVL_WARNING,"Config missing: %s. Using defaults.",filename);
        return;
    }

    char line[128];
    while(fgets(line,sizeof(line),file)) {
            char* actionName = strtok(line," =\n\r\t");
            char* keyName    = strtok(NULL," =\n\r\t");

            if (!actionName || !keyName) continue;

            GameAction action = GetActionFromStr(actionName);
            if (action == -1) continue;

            int key =  GetKeyFromStr(keyName);
            if (key == 0) continue;

            Input_Remap(action,key);
    }
    fclose(file);
    Log(LOG_LVL_INFO,"Config loaded successfully.");
}

void Input_Init(const char* configPath) {
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

    // do not forget
    Input_LoadConfig(configPath);
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









