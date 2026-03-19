#ifndef CRE_INPUT_H
#define CRE_INPUT_H
#include <stdbool.h>
#include <stdint.h>

// use enum classes. Again.
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

  ACTION_COUNT,
  ACTION_INVALID = 255 // This is temporary fix.
} GameAction;

void Input_Init(const char *configPath);

bool Input_IsPressed(GameAction action);

bool Input_IsDown(GameAction action);

void Input_Remap(GameAction action, int32_t key);

void Input_Poll(void);

#endif
