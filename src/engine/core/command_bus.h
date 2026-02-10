#ifndef COMMAND_BUS_H
#define COMMAND_BUS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#include <stdalign.h>

// MSVC needs __restrict instead of restrict
#if defined(_MSC_VER)
    #define restrict __restrict
#endif

// Command Flags
#define CMD_PHYS_FLAG_STATIC  (1 << 0) // Bit 0: Wall/Static
#define CMD_PHYS_FLAG_SENSOR  (1 << 1) // Bit 1: Trigger (Future proofing)
#define CMD_PHYS_FLAG_BULLET  (1 << 2) // Bit 2: CCD/Fast Mover (Future proofing)

// Animation Command Flags
#define ANIM_FLAG_FORCE_RESET  (1 << 0) // Bit 0: 1 = Force restart
#define ANIM_FLAG_LOOP_OVERRIDE (1 << 1) // Bit 1: 1 = Force loop (example)

// ============================================================================
// Configuration
// ============================================================================

#define CMD_BUFFER_SIZE 16384
#define CMD_BUFFER_MASK (CMD_BUFFER_SIZE - 1)

// Compile-time verification: buffer size must be power of 2
static_assert((CMD_BUFFER_SIZE & CMD_BUFFER_MASK) == 0, 
              "CMD_BUFFER_SIZE must be a power of 2");

// ============================================================================
// Command Types
// ============================================================================

typedef enum CommandType {
    CMD_NONE = 0,
    
    // Physics commands
    CMD_PHYS_MOVE,
    CMD_PHYS_APPLY_FORCE,
    CMD_PHYS_SET_VELOCITY,
    CMD_PHYS_LOAD_STATIC,
    CMD_PHYS_DEFINE,
    CMD_PHYS_RESET,
    CMD_PHYS_DEBUG_DRAW,

    // Animation commands
    CMD_ANIM_PLAY,
    CMD_ANIM_STOP,
    CMD_ANIM_PAUSE,
    
    // Audio commands
    CMD_AUDIO_PLAY,
    CMD_AUDIO_STOP,
    
    // Entity commands
    CMD_ENTITY_SPAWN,
    CMD_ENTITY_DESTROY,
    CMD_ENTITY_ADD_COMPONENT,
    CMD_ENTITY_SET_PIVOT,
    
    CMD_TYPE_COUNT
} CommandType;

// ============================================================================
// Command Payload Structures (4-byte aligned)
// ============================================================================

typedef struct {
    float x;
    float y;
} CommandPayloadMove;

typedef struct {
    uint16_t animID;
    uint16_t flags;
} CommandPayloadAnim;

typedef struct {
    float forceX;
    float forceY;
} CommandPayloadForce;

typedef struct {
    uint8_t material_id; // e.g., MAT_WOOF
    uint8_t flags;      // e.g., static
    uint8_t _padding[2];
    float drag;
} CommandPayloadPhysDef;

typedef struct {
    uint16_t soundID;
    uint16_t _pad;
    float volume;
} CommandPayloadAudio;

typedef struct {
    uint32_t type; // Entity type
    float x;
    float y;
} CommandPayloadSpawn;

// ============================================================================
// Command Structure (32 bytes, C11 Anonymous Union)
// ============================================================================

typedef struct Command {
    uint16_t type;          // CommandType (2 bytes)
    uint16_t _reserved;     // Padding/future use (2 bytes)
    uint32_t entityID;      // Target entity (4 bytes)
    
    // Anonymous union - access directly: cmd.move.x, cmd.anim.animID
    union {
        CommandPayloadMove     move;
        CommandPayloadAnim     anim;
        CommandPayloadForce    force;
        CommandPayloadAudio    audio;
        CommandPayloadPhysDef  physDef;
        CommandPayloadSpawn    spawn;
        alignas(4) uint8_t     raw[24];
    };
} Command;

// Compile-time struct verification
static_assert(sizeof(Command) == 32, "Command struct must be exactly 32 bytes");
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
} CommandBus;

// ============================================================================
// Cold Path Functions (Implemented in command_bus.c)
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
static inline CommandIterator CommandBus_GetIterator(const CommandBus* bus) {
    assert(bus != NULL && "CommandBus_GetIterator: bus is NULL");
    
    return (CommandIterator){
        .current = bus->tail,
        .end = bus->head
    };
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