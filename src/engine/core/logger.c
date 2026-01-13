#include "logger.h"
#include "raylib.h"
#include <stdio.h>
#include <stdarg.h> // For "..." arguments
#include <time.h> // For timestamps

// Rewrite some of the code for android compability later on!!!

void RaylibLogHook(int logLevel, const char *text, va_list args) {
// Map Raylib's levels to engine's levels.
    LogLevel myLevel;
    switch (logLevel) {
        case LOG_DEBUG:   myLevel = LOG_LVL_DEBUG; break; 
        case LOG_INFO:    myLevel = LOG_LVL_INFO; break;
        case LOG_WARNING: myLevel = LOG_LVL_WARNING; break;
        case LOG_ERROR:   myLevel = LOG_LVL_ERROR; break;
        case LOG_FATAL:   myLevel = LOG_LVL_ERROR; break;
        default: return; // Ignore "Trace" or "All" spam.
    }

    char buffer[1024];

    // Print raylib's message to engine's buffer.
    vsnprintf(buffer,sizeof(buffer),text,args);

    // Send it to the engine
    Log(myLevel, "[RAYLIB] %s",buffer);
}
static FILE* logFile = NULL;

// Helper function to create a directory
// Deleted MakeDirectory function, instead using raylib's MakeDirectory

void Logger_Init(void) {
    // Get file directory
    const char* appDir = GetApplicationDirectory();
    
    // Create logs folder
    const char* logDir = TextFormat("%slogs",appDir);

    if (!DirectoryExists(logDir)) {
        MakeDirectory(logDir);
    }
    // Build the full filename
    const char* filePath = TextFormat("%s/game.log",logDir);

    logFile = fopen(filePath,"a");
    if (logFile) {
        Log(LOG_LVL_INFO, "Logger Initialized.");
    }
    else {
        // If error , try to save in root folder
        logFile = fopen("game_fallback.log","w");
        printf("ERROR: Could not create log file in logs folder. Trying fallback.\n");
    }

    if (logFile) {
        Log(LOG_LVL_INFO,"Logger Initialized.");

        // For raylib's logs. There are safety warnings here.
        SetTraceLogCallback(RaylibLogHook);
    }
} 
void Logger_Shutdown(void) {
    if (logFile){
        Log(LOG_LVL_INFO, "---- SYSTEM SHUTDOWN ----");
        fclose(logFile);
        logFile = NULL;
    }
}
void Log(LogLevel level, const char* fmt, ...) {
    // get current time
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char timeStr[24];
    strftime(timeStr, sizeof(timeStr), "%d/%m/%Y %H:%M:%S",t);

    const char* tag;
    switch (level) {
        case LOG_LVL_INFO:     tag = "[INFO]";    break;
        case LOG_LVL_WARNING:  tag = "[WARNING]"; break;
        case LOG_LVL_ERROR:    tag = "[ERROR]";   break;
        case LOG_LVL_DEBUG:    tag = "[DEBUG]";   break;
        default:               tag = "[LOG]";     break;
    }
    
    // Handle variable arguments (...)
    va_list args;


    // Write to the terminal
    // This prints "17:30:05 [INFO]"
    printf("%s %s ", timeStr, tag);

    // This prints the actual message : "Game started"
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf("\n");

    // Write to file (game.log)
    if (logFile) {
        fprintf(logFile, "%s %s ", timeStr, tag);

        va_start(args,fmt);
        vfprintf(logFile,fmt,args);
        va_end(args);

        fprintf(logFile,"\n");

        // Force save only if error or warning (can change that later on maybe)
        if (level == LOG_LVL_ERROR || level == LOG_LVL_WARNING) {
            fflush(logFile);
        }
    }
}




