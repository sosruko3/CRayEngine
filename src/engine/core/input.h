#ifndef INPUT_H
#define INPUT_H
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

    ACTION_COUNT
} GameAction;

void Input_Init(void);

bool Input_IsPressed(GameAction action);

bool Input_IsDown(GameAction action);

void Input_Remap(GameAction action,int key);


#endif