#ifndef LOG_H
#define LOG_H

#define LogMsg(...) Log(LOG_INFO, __VA_ARGS__)
#define LogWarning(...) Log(LOG_WARNING, __VA_ARGS__)
#define LogError(...) Log(LOG_ERROR, __VA_ARGS__)
#define AlertMsg(title, ...) LogMsg(__VA_ARGS__); Popup(LOG_INFO, title, __VA_ARGS__)
#define AlertWarning(title, ...) LogWarning(__VA_ARGS__); Popup(LOG_WARNING, title, __VA_ARGS__)
#define AlertError(title, ...) LogError(__VA_ARGS__); Popup(LOG_ERROR, title, __VA_ARGS__)

enum LogLevel {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
};

void Log(int level, const char *fmt, ...);
void Popup(int level, const char *title, const char *fmt, ...);

/*
this should check to see if a log file is already created
if there is one it should:
- check the file size and truncate it if it is over a certain threshold
- check some version info and delete old versions outright
else:
- create a new log file with the current version info
when truncating make sure to preserve the log file version
*/
// int LogFileInit(CE_Arena *arena, const char *path);

#endif
