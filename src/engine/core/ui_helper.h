#ifndef UI_HELPER_H
#define UI_HELPER_H

#include "raylib.h"

void DrawTextCentered(const char* text, int y , int fontSize, Color color);

// Optional: Useful if you want to center text inside a specific box (like a button)
void DrawTextCenteredInBox(const char* text, Rectangle box, int fontSize, Color color);

#endif