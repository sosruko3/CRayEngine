#ifndef CRE_VIEWPORT_H
#define CRE_VIEWPORT_H

#include "stdbool.h"

typedef struct ViewportSize {
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