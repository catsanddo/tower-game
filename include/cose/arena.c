#include "arena.h"
#include "memory.h"

CE_Arena *CE_ArenaAlloc(void)
{
    CE_Arena *result = 0;

    void *memory = CE_MemReserve(CE_ARENA_MAX);
    if (memory != NULL) {
        CE_MemCommit(memory, CE_ARENA_INITIAL);
        result = (CE_Arena *) memory;
        result->base = memory;
        result->offset = CE_ARENA_SIZE;
        result->cap = CE_ARENA_INITIAL;
        result->align = 8;
    }

    return result;
}

void CE_ArenaFree(CE_Arena *arena)
{
    if (!arena || !arena->base) {
        return;
    }
    arena->offset = arena->cap = 0;
    CE_MemRelease(arena->base, CE_ARENA_MAX);
}

void *CE_ArenaPush(CE_Arena *arena, CE_u64 size)
{
    unsigned char *result = 0;

    size = CE_AlignPow2(size, arena->align);

    if (arena->offset + size <= arena->cap) {
        result = arena->base + arena->offset;
        arena->offset += size;
    } else if (arena->offset + size > CE_ARENA_MAX) {
        return 0;
    } else {
        arena->cap <<= 1;
        CE_MemCommit(arena->base, arena->cap);
        return CE_ArenaPush(arena, size);
    }

    return result;
}

void CE_ArenaPop(CE_Arena *arena, CE_u64 size)
{
    size = CE_AlignPow2(size, arena->align);
    arena->offset = CE_Max(arena->offset - size, CE_ARENA_SIZE);
}

CE_u64 CE_ArenaGetPos(CE_Arena *arena)
{
    return CE_Max(arena->offset, CE_ARENA_SIZE);
}
