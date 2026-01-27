#include "command_bus.h"
#include <string.h>

// ============================================================================
// Cold Path Functions
// ============================================================================

void CommandBus_Init(CommandBus* bus) {
    assert(bus != NULL && "Cannot initialize a NULL bus");
    
    bus->head = 0;
    bus->tail = 0;
    
    // Zero out the buffer for clean state
    memset(bus->buffer, 0, sizeof(bus->buffer));
}

void CommandBus_Flush(CommandBus* bus, const CommandIterator* iter) {
    assert(bus != NULL && "Cannot flush NULL bus");
    assert(iter != NULL && "Cannot flush with NULL iterator");
    
    // Advance tail to iterator's end position
    // All commands up to iter->end are now considered consumed
    bus->tail = iter->end;
}

void CommandBus_Clear(CommandBus* bus) {
    assert(bus != NULL);
    
    bus->head = 0;
    bus->tail = 0;
}