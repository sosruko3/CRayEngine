#ifndef CRE_UIHELPER_H
#define CRE_UIHELPER_H

#include "cre_types.h"
#include "raylib.h"  // For Rectangle

void DrawTextCentered(const char* text, int y , int fontSize, creColor color);

// Optional: Useful if you want to center text inside a specific box (like a button)
void DrawTextCenteredInBox(const char* text, Rectangle box, int fontSize, creColor color);

#endif