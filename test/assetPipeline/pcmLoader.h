#ifndef PCMLOADER_H
#define PCMLOADER_H

#include "assetStructs.h"

void LoadPCM(std::vector<audioItem>& items, std::atomic<size_t>& memory_counter, PCMSettings settings);
void FreePCM(std::vector<audioItem>& items);
#endif