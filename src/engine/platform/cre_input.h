#ifndef CRE_INPUT_H
#define CRE_INPUT_H
#include <stdbool.h>

typedef enum {

    ACTION_UP = 0,
    ACTION_DOWN,
    ACTION_LEFT,
    ACTION_RIGHT,

    ACTION_CONFIRM,
    ACTION_BACK,
    ACTION_PAUSE,

    ACTION_PRIMARY,
    ACTION_SECONDARY,
    // ADD NEW ACTIONS, THESE ARE NOT ENOUGH!
    ACTION_COUNT
} GameAction;

void Input_Init(const char* configPath);

bool Input_IsPressed(GameAction action);

bool Input_IsDown(GameAction action);

void Input_Remap(GameAction action,int key);

void Input_Poll(void);

#endif