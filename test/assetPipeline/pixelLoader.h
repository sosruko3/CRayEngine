#ifndef PIXELLOADER_H
#define PIXELLOADER_H

#include "assetStructs.h"

void LoadPixels(std::vector<imageItem>& items,std::atomic<size_t>& memory_counter, PixelSettings settings);
void FreePixels(std::vector<imageItem>& items);
#endif