#include "log.h"
#include "SDL2/SDL.h"
#include <stdio.h>
#include <stdarg.h>

void Log(int level, const char *fmt, ...)
{
    char *log_level[] = {"[INFO]: ", "[WARN]: ", "[ERROR]: "};
    va_list args;
    
#ifdef LOG_FILE
    FILE *log_file = fopen(LOG_FILE, "a");
    va_start(args, fmt);

    if (log_file) {
        fprintf(log_file, "%s", log_level[level]);
        vfprintf(log_file, fmt, args);
        fprintf(log_file, "\n");
        fclose(log_file);
    }
    va_end(args);
#endif
    
    va_start(args, fmt);
    fprintf(stderr, "%s", log_level[level]);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}

void Popup(int level, const char *title, const char *fmt, ...)
{
    char *message = 0;
    CE_u32 flags[] = {SDL_MESSAGEBOX_INFORMATION, SDL_MESSAGEBOX_WARNING, SDL_MESSAGEBOX_ERROR};

    // TODO: accept arena? accept window?
    va_list args;
    va_start(args, fmt);
    CE_u64 length = vsnprintf(message, 0, fmt, args) + 1;
    message = malloc(length);
    va_end(args);
    va_start(args, fmt);
    vsnprintf(message, length, fmt, args);
    va_end(args);

    SDL_ShowSimpleMessageBox(flags[level], title, message, 0);
    free(message);
}
