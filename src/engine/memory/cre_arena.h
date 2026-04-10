#ifndef CRE_ARENA_H
#define CRE_ARENA_H

#include "engine/core/cre_types.h"
#include <stdint.h>
#include <stddef.h>
#include <assert.h>

constexpr size_t DEFAULT_ARENA_ALIGNMENT = 8;
constexpr size_t MASTER_ARENA_ALIGNMENT = 64;

// aligned_alloc wrapper.
Arena arena_AllocateMemory(size_t capacity);
// This is for masterArena only! Do not try to free subArenas!!!
void arena_FreeMemory(Arena* arena);

// Initialize with masterArena, and clear offset with Clear.(No deleting)
void arena_Init(Arena* arena, void* memory, size_t capacity);
void arena_Clear(Arena* arena);

// Splitting an Arena. Would recommend using parent's alignment for the parameter.
Arena arena_Split(Arena* parent, size_t split_size,size_t alignment);
// Save an offset and rewind it.
size_t arena_Mark(const Arena* arena);
void arena_Rewind(Arena* arena, size_t snapshot);

// Remaining bytes of arena
size_t arena_Remaining(const Arena* arena);

// Main function. 
void* arena_PushAligned(Arena* arena, size_t size, size_t alignment);

// Lightweight template for QOL.
template <typename T>
T* arena_Push(Arena* arena, size_t count = 1, size_t alignment = alignof(T)) {
    assert(count <= SIZE_MAX/sizeof(T));
    size_t total_size = sizeof(T) * count;
    return static_cast<T*>(arena_PushAligned(arena, total_size, alignment));
}
#endif