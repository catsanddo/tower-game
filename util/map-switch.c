#include <cJSON.c>
#include <cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

char *GetOutFile(const char *path)
{
    int length = strlen(path);
    for (int i = length-1; i >= 0; --i) {
        if (path[i] == '/' || path[i] == '\\') {
            length = strlen(path);
            break;
        }
        if (path[i] == '.' && i == 0) {
            length = strlen(path);
            break;
        } else if (path[i] == '.') {
            length -= 1;
            break;
        }
        length -= 1;
    }
    if (length < 0) {
        length = strlen(path);
    }
    char *result = malloc(length + 5);
    snprintf(result, length+5, "%.*s.cmp", length, path);
    return result;
}

char *GetBase(char *path, char *buffer, size_t buffer_length)
{
    size_t start = 0;
    size_t length = 0;
    size_t path_length = strlen(path);

    for (size_t i = 0; i < path_length; ++i) {
        if (path[i] == '/' || path[i] == '\\') {
            start = i+1;
            length = 0;
            continue;
        }
        if (path[i] == '.') {
            break;
        }
        length += 1;
    }

    if (length > buffer_length) {
        length = buffer_length;
        printf("[WARNING] base got truncated: '%.*s'", (int) length, path+start);
    }
    memcpy(buffer, path+start, length);

    return buffer;
}

uint32_t NtoH(uint32_t n)
{
    uint32_t result = 0;
    result += (n & 0xff000000) >> 24;
    result += (n & 0x00ff0000) >> 8;
    result += (n & 0x0000ff00) << 8;
    result += (n & 0x000000ff) << 24;

    return result;
}

uint32_t GetTileCount(const char *path)
{
    uint32_t result = 0;

    FILE *file = fopen(path, "rb");
    if (!file) {
        printf("No open tiles: '%s'\n", path);
        exit(1);
    }
    fseek(file, 16, SEEK_SET);
    uint32_t width, height;
    fread(&width, 4, 1, file);
    fread(&height, 4, 1, file);

    width = NtoH(width) / 16;
    height = NtoH(height) / 16;

    result = width * height;
    return result;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Provide path\n");
        return 1;
    }

    char *in_file = argv[1];
    char *out_file = GetOutFile(in_file);

    cJSON *json;
    {
        FILE *file = fopen(in_file, "r");
        if (!file) {
            printf("No open: '%s'\n", in_file);
            return 1;
        }
        char *buffer;
        size_t length;
        fseek(file, 0, SEEK_END);
        length = ftell(file);
        rewind(file);
        buffer = malloc(length);
        fread(buffer, 1, length, file);
        fclose(file);
        json = cJSON_ParseWithLength(buffer, length);
        free(buffer);
    }

    FILE *file = fopen(out_file, "wb");
    if (!file) {
        printf("No open (w): '%s'\n", out_file);
        return 1;
    }

    uint32_t width;
    uint32_t height;
    char tile_sheet[16] = {0};
    uint32_t entity_count = 0;
    uint32_t tiles_count;
    uint32_t color = 0xAAAAAAAAu;
    
    width = cJSON_GetObjectItem(json, "width")->valueint;
    height = cJSON_GetObjectItem(json, "height")->valueint;
    
    char *ts_path = cJSON_GetObjectItem(json, "tileSheet")->valuestring;
    GetBase(ts_path, tile_sheet, 16);

    tiles_count = GetTileCount(ts_path);
    if (tiles_count > 0x7fff) {
        printf("[WARNING] too many tiles (%u)\n", tiles_count);
    }

    cJSON *tile_map = cJSON_GetObjectItem(json, "tileMap");

    // File writing
    char magic[4] = {'C', 'M', 'P', 0x5a};
    fwrite(magic, 4, 1, file);
    fwrite(&width, 4, 1, file);
    fwrite(&height, 4, 1, file);
    fwrite(&tile_sheet, 1, 16, file);
    fwrite(&entity_count, 4, 1, file);
    fwrite(&tiles_count, 4, 1, file);
    
    // Placeholder colors
    fwrite(&color, 4, 1, file);
    fwrite(&color, 4, 1, file);

    cJSON *elem;
    cJSON_ArrayForEach(elem, tile_map) {
        uint32_t raw = elem->valueint;
        if (raw == INT_MAX) {
            raw = (uint32_t) elem->valuedouble;
        }
        uint16_t tile_index = (raw & 0xffff);
        if (raw & 0x80000000) {
            tile_index |= 0x8000;
        }
        fwrite(&tile_index, 2, 1, file);
    }

    // Entities would go here

    // Filling zeros for tile types
    for (uint32_t i = 0; i < tiles_count; ++i) {
        char c = 0;
        fwrite(&c, 1, 1, file);
    }

    fclose(file);

    return 0;
}
