#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "stdbool.h"

typedef struct {
    float width;
    float height;
    float aspect;
} ViewportSize;

void Viewport_Init(int windowWidth,int windowHeight,float targetHeight);
ViewportSize Viewport_Get(void);

bool Viewport_ShouldResize(void);
#endif