#ifndef CE_ARENA_H
#define CE_ARENA_H

#include <stdint.h>
#include <stddef.h>

#define CE_ARENA_MAX (32L << 20)
#define CE_ARENA_INITIAL (4L << 10)
#define CE_ARENA_SIZE (CE_AlignPow2(sizeof(CE_Arena), 8))

#define CE_ArenaClear(a) CE_ArenaPop(a, (a)->offset)

typedef struct CE_Arena CE_Arena;
struct CE_Arena {
    uint8_t *base;
    CE_u64 offset;
    CE_u64 cap;
    CE_u64 align;
};

CE_Arena *CE_ArenaAlloc(void);
void CE_ArenaFree(CE_Arena *arena);
void *CE_ArenaPush(CE_Arena *arena, CE_u64 size);
void CE_ArenaPop(CE_Arena *arena, CE_u64 size);
CE_u64 CE_ArenaGetPos(CE_Arena *arena);

#endif
