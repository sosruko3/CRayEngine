#ifndef CRE_COMMANDBUS_H
#define CRE_COMMANDBUS_H

#include "cre_commandBus_defs.h"
#include "engine/core/cre_types.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#include <stdalign.h>


// MSVC needs __restrict instead of restrict
#if defined(_MSC_VER)
    #define restrict __restrict
#endif

// ============================================================================
// Configuration
// ============================================================================

#define CMD_BUFFER_SIZE 16384
#define CMD_BUFFER_MASK (CMD_BUFFER_SIZE - 1)

#define BUS_PHASE_OPEN 0
#define BUS_PHASE_SIMULATION 1
#define BUS_PHASE_RENDER 2

// Compile-time verification: buffer size must be power of 2
static_assert((CMD_BUFFER_SIZE & CMD_BUFFER_MASK) == 0, 
              "CMD_BUFFER_SIZE must be a power of 2");


// ============================================================================
// Command Structure (64 bytes, C11 Anonymous Union)
// ============================================================================

typedef struct Command {
    uint16_t type;          // CommandType (2 bytes)
    uint16_t _reserved;     // Padding/future use (2 bytes)
    uint32_t _reserved2;    // For 8 byte header.
    Entity entity;          // Target entity(id,generations) (8 bytes)
    
    // Anonymous union - access directly: cmd.move.x, cmd.anim.animID
    union {
        CommandPayloadVec2        vec2;
        CommandPayloadF32         f32;
        CommandPayloadU16         u16;
        CommandPayloadB8          b8;
        CommandPayloadU64         u64;

        CommandPayloadAnim        anim;
        CommandPayloadAudio       audio;
        CommandPayloadPhysDef     physDef;
        CommandPayloadEntityClone entityClone;
        CommandPayloadRenderDepth renderDepth;
        alignas(4) uint8_t     raw[48];
    };
} Command;

// Compile-time struct verification
static_assert(sizeof(Command) == 64, "Command struct must be exactly 64 bytes");
static_assert(alignof(Command) >= 4, "Command struct must be at least 4-byte aligned");

// ============================================================================
// Command Iterator (Zero-Copy Snapshot Pattern)
// ============================================================================

typedef struct CommandIterator {
    uint32_t current;   // Current read index (free-running)
    uint32_t end;       // Snapshot of head at iterator creation
} CommandIterator;

// ============================================================================
// Command Bus (Ring Buffer)
// ============================================================================

typedef struct CommandBus {
    alignas(64) Command buffer[CMD_BUFFER_SIZE];  // Cache-line aligned
    uint32_t head;  // Write index (free-running)
    uint32_t tail;  // Read index (free-running)
    uint32_t consumed_end; // Last snapshot boundary consumed by systems
#ifndef NDEBUG
    uint8_t current_phase;
    uint16_t debug_forbidden_domain;
#endif
} CommandBus;

// ============================================================================
// Cold Path Functions (Implemented in cre_commandBus.c)
// ============================================================================

/**
 * Initialize the command bus, resetting all state.
 * @param bus Pointer to the CommandBus to initialize
 */
void CommandBus_Init(CommandBus* bus);

/**
 * Flush processed commands from the buffer.
 * Sets tail to the iterator's end position.
 * 
 * @param bus Pointer to the CommandBus
 * @param iter Pointer to the completed iterator
 */
void CommandBus_Flush(CommandBus* bus, const CommandIterator* iter);

/**
 * Clear the command bus, resetting head and tail to 0.
 * @param bus Pointer to the CommandBus
 */
void CommandBus_Clear(CommandBus* bus);

// ============================================================================
// Hot Path Inline Functions
// ============================================================================

/**
 * Push a command onto the bus.
 * HOT PATH - inlined for zero function call overhead.
 * 
 * @param bus Pointer to the CommandBus
 * @param cmd The command to push
 * @return true if successful, false if buffer is full
 */
static inline bool CommandBus_Push(CommandBus* restrict bus, Command cmd) {
    assert(bus != NULL && "CommandBus_Push: bus is NULL");

#ifndef NDEBUG
    const uint16_t domain = (uint16_t)(cmd.type & CMD_DOMAIN_MASK);
    if (bus->current_phase == BUS_PHASE_RENDER) {
        assert(domain == CMD_DOMAIN_RENDER || cmd.type == CMD_NONE);
    }
    assert(bus->debug_forbidden_domain == 0 || domain != bus->debug_forbidden_domain);
#endif
    
    // Check if buffer is full (free-running index arithmetic)
    if ((bus->head - bus->tail) >= CMD_BUFFER_SIZE) {
        return false;
    }
    
    // Bitwise AND mask for array index
    const uint32_t index = bus->head & CMD_BUFFER_MASK;
    bus->buffer[index] = cmd;
    
    // Increment head (free-running, natural overflow)
    bus->head++;
    
    return true;
}

/**
 * Create a snapshot iterator for zero-copy command processing.
 * HOT PATH - inlined for zero function call overhead.
 * 
 * @param bus Pointer to the CommandBus
 * @return CommandIterator positioned at tail, ending at current head
 */
static inline CommandIterator CommandBus_GetIterator(CommandBus* bus) {
    assert(bus != NULL && "CommandBus_GetIterator: bus is NULL");
    
    CommandIterator iter = (CommandIterator){
        .current = bus->tail,
        .end = bus->head
    };

    bus->consumed_end = iter.end;
    return iter;
}

/**
 * Advance the iterator and get a direct const pointer to the next command.
 * HOT PATH - inlined for zero function call overhead.
 * 
 * Zero-copy: returns a const pointer directly into the ring buffer.
 * The buffer is immutable to readers.
 * 
 * @param bus Pointer to the CommandBus
 * @param iter Pointer to the iterator to advance
 * @param outPtr Receives const pointer to command in buffer (zero-copy, read-only)
 * @return true if command available, false if iterator exhausted
 */
static inline bool CommandBus_Next(const CommandBus* bus, 
                                   CommandIterator* iter, 
                                   const Command** outPtr) {
    assert(bus != NULL && "CommandBus_Next: bus is NULL");
    assert(iter != NULL && "CommandBus_Next: iter is NULL");
    assert(outPtr != NULL && "CommandBus_Next: outPtr is NULL");
    
    // Check if iterator is exhausted
    if (iter->current == iter->end) {
        *outPtr = NULL;
        return false;
    }
    
    // Get const pointer to command in buffer (zero-copy, immutable)
    const uint32_t index = iter->current & CMD_BUFFER_MASK;
    *outPtr = &bus->buffer[index];
    
    // Advance iterator (free-running)
    iter->current++;
    
    return true;
}

// ============================================================================
// Inline Utility Functions
// ============================================================================

static inline uint32_t CommandBus_Count(const CommandBus* bus) {
    assert(bus != NULL);
    return bus->head - bus->tail;
}

static inline bool CommandBus_IsEmpty(const CommandBus* bus) {
    assert(bus != NULL);
    return bus->head == bus->tail;
}

static inline bool CommandBus_IsFull(const CommandBus* bus) {
    assert(bus != NULL);
    return (bus->head - bus->tail) >= CMD_BUFFER_SIZE;
}

static inline uint32_t CommandIterator_Remaining(const CommandIterator* iter) {
    assert(iter != NULL);
    return iter->end - iter->current;
}

static inline bool CommandIterator_IsDone(const CommandIterator* iter) {
    assert(iter != NULL);
    return iter->current == iter->end;
}

#endif