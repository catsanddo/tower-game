#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

enum EntityFlags {
    NONE = 0,
    PLAYER = 1,
    VISIBLE = 2,
    ACTIVE = 4,
    MOVE = 8,
    ANIMATE = 16,
    SOLID = 32,
    INTERACT = 128,
    TRIGGER = 256,
};

typedef struct Entity Entity;
struct Entity {
    char *name;
    int framerate;
    uint32_t flags;
    int position[2];
    uint32_t eventID;
};

Entity table[] = {
    { "res/sprites/Sword Man.png",  3, VISIBLE|ANIMATE|SOLID|INTERACT, { 15, 35 }, 11 },
    { "res/sprites/Ghost Girl.png", 3, VISIBLE|ANIMATE|SOLID|INTERACT, { 14, 11 }, 12 },
    { "res/sprites/Skuller.png",    4, VISIBLE|ANIMATE|SOLID|INTERACT, { 26, 11 }, 13 },
    { "res/sprites/Bug Man.png",    1, VISIBLE|ANIMATE|SOLID|INTERACT, { 39, 12 }, 14 },
    { "res/sprites/Dream Worm.png", 1, VISIBLE|ANIMATE|SOLID|INTERACT, { 33, 42 }, 15 },
    { "res/sprites/Mook.png",       2, VISIBLE|ANIMATE|SOLID|INTERACT, {  7, 15 }, 16 },
    { "res/sprites/The Seer.png",   3, VISIBLE|ANIMATE|SOLID|INTERACT, {  4, 1  }, 17 },
    { "res/sprites/Orb Placed.png", 0, VISIBLE|SOLID|INTERACT,         { 20, 19 }, 20 },
    { "res/sprites/Door.png",       0, VISIBLE|SOLID,                  { 31, 13 }, 21 },
    { "res/sprites/Lever.png",      0, VISIBLE|SOLID|INTERACT,         { 23, 1  }, 22 },
    { "res/sprites/Lever.png",      0, VISIBLE|SOLID|INTERACT,         { 24, 1  }, 22 },
    { "res/sprites/Lever.png",      0, VISIBLE|SOLID|INTERACT,         { 25, 1  }, 22 },
    { "res/sprites/Lever.png",      0, VISIBLE|SOLID|INTERACT,         { 26, 1  }, 22 },
    { "res/sprites/Fruit.png",      0, VISIBLE|SOLID|INTERACT,         { 15, 29 }, 23 },

    // Forest triggers
    { "res/sprites/Blank.png",      0, TRIGGER,                        { 78,  8 }, 100 },
    { "res/sprites/Blank.png",      0, TRIGGER,                        { 79, 8 }, 100 },
    { "res/sprites/Blank.png",      0, TRIGGER,                        { 163, 124 }, 100 },
    { "res/sprites/Blank.png",      0, TRIGGER,                        { 79, 156 }, 100 },
    { "res/sprites/Blank.png",      0, TRIGGER,                        { 78, 156 }, 100 },
    { "res/sprites/Blank.png",      0, TRIGGER,                        {  8, 83 }, 100 },

    // Town triggers
    { "res/sprites/Blank.png",      0, TRIGGER,                        { 16, 43 }, 101 },
    { "res/sprites/Blank.png",      0, TRIGGER,                        { 17, 43 }, 101 },
    { "res/sprites/Blank.png",      0, TRIGGER,                        { 26, 11 }, 101 },

    // Orb Cave triggers
    { "res/sprites/Blank.png",      0, TRIGGER,                        {  4, 25 }, 102 },
    { "res/sprites/Blank.png",      0, TRIGGER,                        { 25, 29 }, 102 },

    // Sun Cave triggers
    { "res/sprites/Blank.png",      0, TRIGGER,                        { 23, 32 }, 103 },

    // Beach triggers
    { "res/sprites/Blank.png",      0, TRIGGER,                        { 21, 10 }, 104 },
    { "res/sprites/Blank.png",      0, TRIGGER,                        { 22, 10 }, 104 },
    { "res/sprites/Blank.png",      0, TRIGGER,                        { 35, 27 }, 104 },
    { "res/sprites/Blank.png",      0, TRIGGER,                        { 36, 27 }, 104 },
    { "res/sprites/Blank.png",      0, TRIGGER,                        { 21, 52 }, 104 },

    // Swamp triggers
    { "res/sprites/Blank.png",      0, TRIGGER,                        { 37, 41 }, 105 },
    { "res/sprites/Blank.png",      0, TRIGGER,                        { 38, 41 }, 105 },
    { "res/sprites/Blank.png",      0, TRIGGER,                        { 66,  8 }, 105 },

    // Ruins triggers
    { "res/sprites/Blank.png",      0, TRIGGER,                        { 20,  4 }, 106 },

    // Tower Cave triggers
    { "res/sprites/Blank.png",      0, TRIGGER,                        { 7,  20 }, 107 },
    { "res/sprites/Blank.png",      0, TRIGGER,                        { 7,   4 }, 107 },

    // Tower Base triggers
    { "res/sprites/Blank.png",      0, TRIGGER,                        { 14,  28 }, 108 },
    { "res/sprites/Blank.png",      0, TRIGGER,                        { 15,  13 }, 108 },

    // Tower triggers
    { "res/sprites/Blank.png",      0, TRIGGER,                        { 31,  53 }, 109 },
    { "res/sprites/Blank.png",      0, TRIGGER,                        { 58,  51 }, 109 },

    // Tower Top triggers
    { "res/sprites/Blank.png",      0, TRIGGER,                        { 11,  12 }, 110 },
    { "res/sprites/Blank.png",      0, TRIGGER,                        { 12,  12 }, 201 },

    { "res/sprites/Orb.png",        0, VISIBLE|SOLID|INTERACT,         { 31,  8 }, 24 },

    { "res/sprites/Blank.png",        0, TRIGGER,                        { 66,  107 }, 200 },
};

void WritePad(FILE *file, size_t padding)
{
    uint64_t zero = 0;
    while (padding > 8) {
        fwrite(&zero, 1, 8, file);
        padding -= 8;
    }
    fwrite(&zero, 1, padding, file);
}

void WriteOffset(FILE *file, uint32_t offset, size_t index)
{
    long cur_offset = ftell(file);
    fseek(file, 8 + index * 4, SEEK_SET);
    fwrite(&offset, 4, 1, file);
    fseek(file, cur_offset, SEEK_SET);
}

int main(void)
{
    FILE *file = fopen("entity.dat", "wb");
    if (!file) {
        perror("Couldn't open file");
        return 1;
    }

    uint64_t entity_count = sizeof(table) / sizeof(Entity);
    fwrite(&entity_count, 8, 1, file);
    for (size_t i = 0; i < entity_count; ++i) {
        WritePad(file, 4);
    }
    if (ftell(file) & (8-1)) {
        WritePad(file, 4);
    }

    /* Entity Entry:
     * 32b signed x position
     * 32b signed y position
     * 32b flags
     * 32b eventID
     * 32b framerate
     * 64b name_length
     * string name
     */
    for (size_t i = 0; i < entity_count; ++i) {
        uint32_t offset = ftell(file);
        uint64_t name_length = strlen(table[i].name);
        fwrite(&table[i].position[0], 4, 1, file);
        fwrite(&table[i].position[1], 4, 1, file);
        fwrite(&table[i].flags, 4, 1, file);
        fwrite(&table[i].eventID, 4, 1, file);
        fwrite(&table[i].framerate, 4, 1, file);
        fwrite(&name_length, 8, 1, file);
        fwrite(table[i].name, 1, name_length, file);
        WriteOffset(file, offset, i);

        WritePad(file, 8 - (ftell(file) & 7));
    }

    fclose(file);
    return 0;
}
