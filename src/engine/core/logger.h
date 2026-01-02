#ifndef LOGGER_H
#define LOGGER_H

typedef enum {
    LOG_LVL_INFO, // Normal things
    LOG_LVL_WARNING, // Low-priority problems.
    LOG_LVL_ERROR, // High-priority problems.
    LOG_LVL_DEBUG // For dev.
} LogLevel;

void Logger_Init(void);

void Logger_Shutdown(void);

void Log(LogLevel level,const char* fmt, ...);

#endif