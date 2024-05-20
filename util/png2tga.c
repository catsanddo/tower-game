#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
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
    snprintf(result, length+5, "%.*s.tga", length, path);
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

typedef struct Color Color;
struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

Color color_map[1024];
size_t color_map_size = 0;

int CmpColor(Color lhs, Color rhs)
{
    uint32_t a = *((uint32_t *) &lhs);
    uint32_t b = *((uint32_t *) &rhs);
    return a == b;
}

int MapColor(Color c)
{
    if (color_map_size > 1024) {
        printf("MapColor: color map is full\n");
        exit(1);
    }

    for (size_t i = 0; i < color_map_size; ++i) {
        if (CmpColor(c, color_map[i])) {
            return i;
        }
    }

    color_map[color_map_size] = c;
    color_map_size += 1;
    return color_map_size-1;
}

void WritePacket(unsigned char *buffer, size_t *buffer_length,
        unsigned char *packet, size_t packet_length, int run)
{
    buffer[*buffer_length] = (run << 7) | ((packet_length - 1) & 0x7f);
    *buffer_length += 1;
    if (run) {
        buffer[*buffer_length] = packet[0];
        *buffer_length += 1;
        return;
    }
    
    memcpy(buffer + *buffer_length, packet, packet_length);
    *buffer_length += packet_length;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Provide path\n");
        return 1;
    }

    char *in_file = argv[1];
    char *out_file = GetOutFile(in_file);

    printf("Writing to '%s'\n", out_file);

    int width, height, n;
    unsigned char *image = stbi_load(in_file, &width, &height, &n, 4);

    // if (!res) {
    //     printf("Write failed!\n");
    // }

    unsigned char *buffer = malloc(width * height * 2);
    size_t buffer_length = 0;

    // for (int y = 0; y < height; ++y) {
    //     size_t packet_start = buffer_length;
    //     int packet_length = 0;
    //     int last_color = -1;
    //     int run = 0;
    //     for (int x = 0; x < width; ++x) {
    //         Color c = *((Color *) &image[(y*width+x)*4]);
    //         int color = MapColor(c);
    //         if (packet_length == 0x80) {
    //             buffer[packet_start] = (run << 7) | 0x7f;
    //             packet_start = buffer_length;
    //             c = *((Color *) &image[(y*width+x+1)*4]);
    //             int next = MapColor(c);
    //             if (color == next) {
    //                 packet_length = 2;
    //                 buffer_length += 2;
    //                 buffer[buffer_length-1] = color;
    //                 last_color = color;
    //                 run = 1;
    //                 x += 1;
    //                 continue;
    //             } else {
    //                 buffer_length += 2;
    //                 buffer[buffer_length-1] = color;
    //                 packet_length = 1;
    //                 last_color = color;
    //                 continue;
    //             }
    //         }
    //         if (color != last_color && !run) {
    //             buffer[buffer_length] = color;
    //             buffer_length += 1;
    //             packet_length += 1;
    //             last_color = color;
    //         } else if (color == last_color && !run) {
    //             buffer[packet_start] = (packet_length - 1) & 0x7f;
    //             packet_start = buffer_length-1;
    //             packet_length = 2;
    //             buffer[buffer_length] = color;
    //             buffer_length += 1;
    //             run = 1;
    //         } else if (color != last_color && run) {
    //             buffer[packet_start] = 0x80 | ((packet_length - 1) & 0x7f);
    //             packet_start = buffer_length;
    //             packet_length = 1;
    //             last_color = color;
    //             buffer[buffer_length+1] = color;
    //             buffer_length += 2;
    //             run = 0;
    //         } else if (color == last_color && run) {
    //             packet_length += 1;
    //         }
    //     }
    //     // TODO
    // }
    
    for (int y = 0; y < height; ++y) {
        unsigned char packet[128];
        size_t packet_length = 0;
        int run;
        int last_color;
        for (int x = 0; x < width; ++x) {
            Color c = *((Color *) &image[(y*width+x) * 4]);
            int color = MapColor(c);

            if (packet_length == 128) {
                WritePacket(buffer, &buffer_length, packet, packet_length, run);
                packet_length = 0;
            }

            if (packet_length == 0) {
                packet[0] = color;
                packet_length += 1;
                c = *((Color *) &image[(y*width+x+1) * 4]);
                int next = MapColor(c);
                if (color == next) {
                    run = 1;
                } else {
                    run = 0;
                }
                last_color = color;
            } else if (color == last_color && run) {
                packet_length += 1;
            } else if (color == last_color && !run) {
                // remove last element; it will part of new packet
                packet_length -= 1;
                WritePacket(buffer, &buffer_length, packet, packet_length, run);
                run = 1;
                packet_length = 2;
                packet[0] = color;
            } else if (color != last_color && run) {
                WritePacket(buffer, &buffer_length, packet, packet_length, run);
                // run = 0;
                packet_length = 0;
                x -= 1;
                // packet[0] = color;
            } else if (color != last_color && !run) {
                packet[packet_length] = color;
                packet_length += 1;
            }
            
            last_color = color;
        }
        WritePacket(buffer, &buffer_length, packet, packet_length, run);
    }

    FILE *file = fopen(out_file, "wb");
    if (!file) {
        printf("Could not write to '%s'\n", out_file);
        return 1;
    }

    uint8_t byte = 0;
    uint16_t word = 0;
    fwrite(&byte, 1, 1, file);
    byte = 1;
    fwrite(&byte, 1, 1, file);
    byte = 9;
    fwrite(&byte, 1, 1, file);
    fwrite(&word, 2, 1, file);
    word = color_map_size;
    fwrite(&word, 2, 1, file);
    byte = 32;
    fwrite(&byte, 1, 1, file);
    word = 0;
    fwrite(&word, 2, 1, file);
    fwrite(&word, 2, 1, file);
    word = width;
    fwrite(&word, 2, 1, file);
    word = height;
    fwrite(&word, 2, 1, file);
    byte = 8;
    fwrite(&byte, 1, 1, file);
    byte = 0x20;
    fwrite(&byte, 1, 1, file);

    for (int i = 0; i < color_map_size; ++i) {
        byte = color_map[i].b;
        fwrite(&byte, 1, 1, file);
        byte = color_map[i].g;
        fwrite(&byte, 1, 1, file);
        byte = color_map[i].r;
        fwrite(&byte, 1, 1, file);
        byte = color_map[i].a;
        fwrite(&byte, 1, 1, file);
    }

    fwrite(buffer, 1, buffer_length, file);
    fclose(file);

    return 0;
}
