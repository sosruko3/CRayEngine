#include "ui_helper.h"
#include "config.h"
#include "types_macro.h"

void DrawTextCentered(const char* text, int y, int fontSize, creColor color)
{
    int textWidth = MeasureText(text,fontSize);
    DrawText(text,(SCREEN_WIDTH - textWidth)/2,y,fontSize,R_COL(color));
}

void DrawTextCenteredInBox(const char* text, Rectangle box, int fontSize, creColor color)
{
    int textWidth = MeasureText(text, fontSize);
    
    // Calculate center of the box
    int centerX = box.x + (box.width / 2) - (textWidth / 2);
    int centerY = box.y + (box.height / 2) - (fontSize / 2); // Approximation for height
    
    DrawText(text, centerX, centerY, fontSize, R_COL(color));
}
