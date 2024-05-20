#ifndef CE_STRING_H
#define CE_STRING_H

#include <stddef.h>
#include "arena.h"

#define CE_StrVArg(s) (int) s.length, s.str
#define CE_StrLit(s) (CE_String8) { (CE_u8 *) (s), sizeof(s)-1 }
#define CE_StrIsNull(s) ((s).str == NULL)
#define CE_StrEndPtr(s) ((s).str + (s).length - 1)

typedef struct CE_String8 CE_String8;
struct CE_String8 {
    CE_u8 *str;
    CE_u64 length;
};

typedef struct CE_String16 CE_String16;
struct CE_String16 {
    CE_u16 *str;
    CE_u64 length;
};

typedef struct CE_String32 CE_String32;
struct CE_String32 {
    CE_u32 *str;
    CE_u64 length;
};

typedef struct CE_String8Node CE_String8Node;
struct CE_String8Node {
    CE_String8 string;
    CE_String8Node *next, *prev;
};

typedef struct CE_String8List CE_String8List;
struct CE_String8List {
    CE_String8Node *first;
    CE_String8Node *last;
    CE_u64 length;
};

CE_String8 CE_StrC(CE_Arena *arena, const char *str);
CE_b32 CE_StrEq(CE_String8 a, CE_String8 b);
CE_String8 CE_StrClone(CE_Arena *arena, CE_String8 src);
CE_String8 CE_StrCat(CE_Arena *arena, CE_String8 lhs, CE_String8 rhs);
CE_String8List CE_StrSplit(CE_Arena *arena, CE_String8 source, CE_String8 separator);
CE_u64 CE_StrFind(CE_String8 source, CE_String8 sub_string);
CE_String8 CE_StrSub(CE_String8 source, CE_u64 a, CE_u64 b);

CE_String16 CE_Str8ToStr16(CE_Arena *arena, CE_String8 source);
CE_String8 CE_Str16ToStr8(CE_Arena *arena, CE_String16 source);
CE_String32 CE_Str8ToStr32(CE_Arena *arena, CE_String8 source);
CE_String8 CE_Str32ToStr8(CE_Arena *arena, CE_String32 source);

#endif
