#include "cre_sys.h"
#include "raylib.h"

const char *Platform_GetAppDir(void) { return GetApplicationDirectory(); }
bool Platform_DirExists(const char *dirPath) {
  return DirectoryExists(dirPath);
}

int Platform_MakeDir(const char *dirPath) {
  int x = MakeDirectory(dirPath);
  if (x)
    return 0;
  return 1;
}
double Platform_GetTime(void) { return GetTime(); }
