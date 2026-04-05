#include "cre_logger.h"
#include "../platform/cre_sys.h"
#include "fmt/base.h"
#include "fmt/chrono.h"
#include <stdio.h>

static constexpr const char *LOG_TAGS[] = {
  "[INFO]" , "[WARNING]",
  "[ERROR]", "[DEBUG]"};

static FILE *logFile = nullptr;
void Logger_WriteToFile(const char *finalMessage, LogLevel level) {
  fputs(finalMessage, stdout);

  if (logFile) {
    fputs(finalMessage, logFile);

    if (level == LogLevel::Warning || level == LogLevel::Error) {
      fflush(logFile);
    }
  }
}

// Might think about making this a bool return type. For safety.
void Logger_Init(void) {
  // Get file directory
  const char *appDir = Platform_GetAppDir();

  char pathBuffer[512] = {};

  // Creates logs folder.
  fmt::format_to_n(pathBuffer, sizeof(pathBuffer) - 1, "{}logs", appDir);

  if (!Platform_DirExists(pathBuffer)) {
    Platform_MakeDir(pathBuffer);
  }
  // Build the full filename
  fmt::format_to_n(pathBuffer, sizeof(pathBuffer) - 1, "{}logs/game.log",
                   appDir);

  logFile = fopen(pathBuffer, "a");
  if (logFile) {
    Log(LogLevel::Info, "Logger Initialized.");
  } else {
    // If error , try to save in root folder
    logFile = fopen("game_fallback.log", "w");
    fmt::print(
        stderr,
        "ERROR: Could not create log file in logs folder. Trying fallback.\n");
  }
}

void Logger_Shutdown(void) {
  if (logFile) {
    Log(LogLevel::Info, "Logger Shutdown.");
    fclose(logFile);
    logFile = nullptr;
  }
}

void Logger_LogImplementation(LogLevel level, fmt::string_view format,
                              fmt::format_args args) {
  char buffer[1024] = {};

  const std::chrono::system_clock::time_point now =
      std::chrono::system_clock::now();

  // DD/MM/YY H:M:S
  // -2 is for \n. In case buffer is full.
    fmt::format_to_n_result<char *> res1 =
      fmt::format_to_n(buffer, sizeof(buffer) - 2, "{:%d/%m/%Y %H:%M:%S} {} ",
                       std::chrono::time_point_cast<std::chrono::seconds>(now), 
                       LOG_TAGS[static_cast<uint8_t>(level)]
                      );

  size_t written = (res1.size > sizeof(buffer) - 2) ? (sizeof(buffer) - 2) : res1.size;
  fmt::format_to_n_result<char *> res2 =
      fmt::vformat_to_n(buffer + written, sizeof(buffer) - 2 - written, format, args);
  fmt::format_to_n(res2.out, 1, "\n");
  Logger_WriteToFile(buffer, level);
}
