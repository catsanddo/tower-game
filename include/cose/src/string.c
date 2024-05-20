#include "string.h"
#include <string.h>

CE_String8 CE_StrC(CE_Arena *arena, const char *str)
{
    CE_u64 length = 0;
    CE_String8 result;

    while (str[length++] != 0);

    result.length = length - 1;
    result.str = CE_ArenaPush(arena, length);
    for (CE_u64 i = 0; i < length; ++i) {
        result.str[i] = str[i];
    }

    return result;
}

CE_b32 CE_StrEq(CE_String8 a, CE_String8 b)
{
    if (a.length != b.length) {
        return 0;
    }
    if (CE_StrIsNull(a) || CE_StrIsNull(b)) {
        if (a.str == b.str) {
            return 1;
        }
        return 0;
    }
    for (CE_u64 i = 0; i < a.length; ++i) {
        if (a.str[i] != b.str[i]) {
            return 0;
        }
    }

    return 1;
}

CE_String8 CE_StrClone(CE_Arena *arena, CE_String8 src)
{
    CE_String8 result;

    result.str = CE_ArenaPush(arena, src.length + 1);

    if (result.str != NULL) {
        result.length = src.length;
        memcpy(result.str, src.str, result.length);
        result.str[result.length] = 0;
    } else {
        result.length = 0;
    }

    return result;
}

CE_String8 CE_StrCat(CE_Arena *arena, CE_String8 lhs, CE_String8 rhs)
{
    CE_String8 result;

    result.str = CE_ArenaPush(arena, lhs.length + rhs.length + 1);

    if (result.str != NULL) {
        result.length = lhs.length + rhs.length;
        memcpy(result.str, lhs.str, result.length);
        memcpy(result.str + lhs.length, rhs.str, result.length);
        result.str[result.length] = 0;
    } else {
        result.length = 0;
    }

    return result;
}

CE_String8List CE_StrSplit(CE_Arena *arena, CE_String8 source, CE_String8 separator)
{
    CE_String8List result = {0};

    CE_String8 window = { source.str, separator.length };
    CE_String8 phrase = { source.str, 0 };

    while (CE_StrEndPtr(window) <= CE_StrEndPtr(source)) {
        if (CE_StrEq(window, separator)) {
            CE_String8Node *node = CE_ArenaPush(arena, sizeof(CE_String8Node));
            node->string = phrase;
            CE_DLLPushEnd(result.first, result.last, node);
            result.length += 1;

            window.str += separator.length;
            phrase.str = window.str;
            phrase.length = 0;
        }
        window.str += 1;
        phrase.length += 1;
    }

    phrase.length += window.length - 1;
    CE_String8Node *node = CE_ArenaPush(arena, sizeof(CE_String8Node));
    node->string = phrase;
    CE_DLLPushEnd(result.first, result.last, node);

    return result;
}

CE_u64 CE_StrFind(CE_String8 source, CE_String8 sub_string)
{
    CE_String8 window = { source.str, sub_string.length };
    
    while (window.str + window.length <= source.str + source.length) {
        if (CE_StrEq(window, sub_string)) {
            return window.str - source.str;
        }
        window.str += 1;
    }

    return source.length;
}

CE_String8 CE_StrSub(CE_String8 source, CE_u64 a, CE_u64 b)
{
    if (a > b || a >= source.length) {
        return (CE_String8) {0};
    }

    b = CE_Min(source.length, b);

    CE_String8 result;
    result.str = &source.str[a];
    result.length = b - a;

    return result;
}

#if 0
CE_String16 CE_Str8ToStr16(CE_Arena *arena, CE_String8 source)
{
}

CE_String8 CE_Str16ToStr8(CE_Arena *arena, CE_String16 source)
{
}

CE_String32 CE_Str8ToStr32(CE_Arena *arena, CE_String8 source)
{
    CE_u64 codepoints = 0;
    for (CE_u64 i = 0; i < source.length; ++i) {
        if (!(source.str[i] & 0x80)) {
            codepoints += 1;
            continue;
        }
        switch ((source.str[i] & 0xf0) >> 4) {
            case 0xf:
                codepoints += 1;
                i += 1;
            case 0xe:
                codepoints += 1;
                i += 1;
            case 0xc:
                codepoints += 1;
                i += 1;
        }
    }

    CE_String32 result;
    result.str = CE_ArenaPush(arena, sizeof(CE_u32) * (codepoints + 1));
    result.length = codepoints;

    CE_u64 i_32 = 0;
    for (CE_u64 i = 0; i < source.length; ++i) {
        CE_u32 rune = 0;
        if (!(source.str[i] & 0x80)) {
            rune = source.str[i];
        } else {
            switch ((source.str[i] & 0xf0) >> 4) {
                case 0xf:
                    rune += source.str[i++] & 0x7;
                    rune <<= 3;
                    rune += source.str[i++] & 0x3f;
                    rune <<= 6;
                    rune += source.str[i++] & 0x3f;
                    rune <<= 6;
                    rune += source.str[i] & 0x3f;
                    rune <<= 6;
                    break;
                case 0xe:
                    rune += source.str[i++] & 0xf;
                    rune <<= 4;
                    rune += source.str[i++] & 0x3f;
                    rune <<= 6;
                    rune += source.str[i] & 0x3f;
                    rune <<= 6;
                    break;
                case 0xc:
                    rune += source.str[i++] & 0x1f;
                    rune <<= 5;
                    rune += source.str[i] & 0x3f;
                    rune <<= 6;
                    break;
            }
        }

        result.str[i_32++] = rune;
    }

    result.str[result.length] = 0;
    return result;
}

CE_String8 CE_Str32ToStr8(CE_Arena *arena, CE_String32 source)
{
}
#endif
