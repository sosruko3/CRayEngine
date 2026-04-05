#ifndef CRE_LOGGER_H
#define CRE_LOGGER_H

#include "fmt/base.h"
#include <stdint.h>
#include <utility>

enum class LogLevel : uint8_t {
    Info = 0,
    Warning,
    Error,
    Debug
};

void Logger_Init(void);
void Logger_Shutdown(void);
void Logger_LogImplementation(LogLevel level, fmt::string_view format, fmt::format_args args);
void Logger_WriteToFile(const char* finalMessage, LogLevel level);

template <typename... Args>
inline void Log(LogLevel level, fmt::format_string<Args...> format, const Args&... args) {
    Logger_LogImplementation(level, format, fmt::make_format_args(args...));
}


#endif
