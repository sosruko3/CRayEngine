#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "stdbool.h"

typedef struct {
    float width;
    float height;
    float aspect;
} ViewportSize;

static void CalculateInternal(int w,int h);
void Viewport_Init(int initialW,int initialH);
ViewportSize Viewport_Get(void);
void Viewport_Update(void);

bool Viewport_wasResized(void);
#endif