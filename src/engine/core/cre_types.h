#ifndef CRE_TYPES_H
#define CRE_TYPES_H
#include <stdint.h>


typedef struct creVec2 {
    float x;
    float y;
} creVec2;

typedef struct creColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} creColor;

typedef struct creRectangle {
    float x;                // Rectangle top-left corner position x
    float y;                // Rectangle top-left corner position y
    float width;            // Rectangle width
    float height;           // Rectangle height
} creRectangle;

#endif