#include "cre_arena.h"

#include <cstdlib>
#include <assert.h>
#include <stdio.h>

Arena arena_AllocateMemory(size_t capacity) {
    assert(capacity > 0);
    assert(capacity <= (SIZE_MAX - (MASTER_ARENA_ALIGNMENT - 1)) && "[ARENA] Requested Allocation size too large!");
    size_t aligned_capacity = (capacity + MASTER_ARENA_ALIGNMENT - 1) & ~(MASTER_ARENA_ALIGNMENT - 1);
    void* raw_memory = std::aligned_alloc(MASTER_ARENA_ALIGNMENT,aligned_capacity);
    assert(raw_memory != nullptr);

    Arena arena = {};
    arena_Init(&arena, raw_memory, aligned_capacity);
    return arena;
}
void arena_FreeMemory(Arena* arena) {   
    if (!arena || !(arena->base_ptr)) return;
    std::free(arena->base_ptr);
    *arena = {};
}

void arena_Init(Arena* arena, void* memory, size_t capacity) {
    assert(arena != nullptr && memory != nullptr);
    arena->base_ptr = static_cast<uint8_t*>(memory);
    arena->capacity = capacity;
    arena->offset   = 0;
}
void arena_Clear(Arena* arena) {
    if (!arena) return;
    arena->offset = 0;
}

size_t arena_Mark(const Arena* arena) {
    assert(arena != nullptr);
    return arena->offset; 
}

void arena_Rewind(Arena* arena, size_t snapshot) {
    assert(arena != nullptr);
    assert(snapshot <= arena->offset && "[ARENA] No forward rewind allowed!");
    arena->offset = snapshot;
}

size_t arena_Remaining(const Arena* arena) {
    assert(arena != nullptr);
    return arena->capacity - arena->offset;
}

Arena arena_Split(Arena* parent, size_t split_size, size_t alignment) {
    void* child_memory = arena_PushAligned(parent, split_size, alignment);
    Arena child_arena;
    arena_Init(&child_arena, child_memory, split_size);
    return child_arena;
}
void* arena_PushAligned(Arena* arena, size_t size, size_t alignment) {
    assert(arena != nullptr);
    assert(alignment != 0 && (alignment & (alignment - 1)) == 0 
    && "Alignment should be power of 2");
    assert(arena->offset <= arena->capacity);

    uintptr_t current_addr = reinterpret_cast<uintptr_t>(arena->base_ptr + arena->offset);

    size_t padding = 0;
    size_t modulo = current_addr & (alignment - 1);
    if (modulo != 0)
        padding = alignment - modulo;
    
    size_t total_needed = size + padding;
    assert(total_needed >= size && "Size_t Overflow detected!");
    assert(total_needed <= (arena->capacity-(arena->offset)) && "Arena is out of memory!");
    void* aligned_memory = reinterpret_cast<void*>(arena->base_ptr + arena->offset + padding);
    arena->offset += total_needed;
    return aligned_memory;
}   