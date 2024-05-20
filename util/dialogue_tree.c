#include <stdio.h>
#include <assert.h>
#include <cose/cose.h>

typedef struct Dialogue Dialogue;
struct Dialogue {
    int order;
    int flags[8];
    int continuation;

    CE_String8 *lines;
    CE_u64 *lines_count;
    
    Dialogue *next;
    Dialogue *prev;
};

typedef struct Character Character;
struct Character {
    int lo_range, hi_range;
    int event;
    Dialogue* first;
    Dialogue* last;
};

int is_digit(char c)
{
    return '0' <= c && c <= '9';
}

CE_u64 parse_int(CE_String8 str, int *num)
{
    assert(num);
    *num = 0;

    CE_u64 i = 0;
    while (i < str.length && is_digit(str.str[i])) {
        *num *= 10;
        *num += str.str[i] - '0';
        i += 1;
    }

    return i;
}

void parse(CE_Arena *arena, Character *ch, CE_String8 src)
{
    int n;
    CE_u64 length;

    assert(ch);
    
    // Grab event
    length = parse_int(src, &n);
    ch->event = n;
    assert(length != 0);
    src.str += length + 1;
    src.length -= length + 1;
    // Grab lo_range
    length = parse_int(src, &n);
    ch->lo_range = n;
    assert(length != 0);
    src.str += length + 1;
    src.length -= length + 1;
    // Grab hi_range
    length = parse_int(src, &n);
    ch->hi_range = n;
    assert(length != 0);
    src.str += length;
    src.length -= length;
}

CE_String8 read_file(CE_Arena *arena, const char *path)
{
    FILE *file = fopen(path, "r");
    CE_String8 result = {0};

    if (!file) {
        return result;
    }

    fseek(file, 0, SEEK_END);
    result.length = ftell(file);
    rewind(file);
    result.str = CE_ArenaPush(arena, result.length);
    fread(result.str, 1, result.length, file);

    fclose(file);
    return result;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: %s <input_file>", argv[0]);
        return 0;
    }

    CE_Arena *arena = CE_ArenaAlloc();
    const char *input_file = argv[1];
    Character character = {0};

    CE_String8 input = read_file(arena, input_file);
    parse(arena, &character, input);

    printf("%d:%d-%d\n", character.event, character.lo_range, character.hi_range);

    CE_ArenaFree(arena);
    return 0;
}

/*
100:300-400
1
#1

2
#1-

3 &ruins
#4

^3
#4-

3 &fruit
#2

^3
#2-

3 &elder
#3

^3
#3-

3 &star
#5

^3
#5-

In dialogue prefixes
! - :PAU event
# - :TCR event
. - end dialogue
*/
