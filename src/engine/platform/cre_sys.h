#ifndef CRE_SYS_H
#define CRE_SYS_H

const char* Platform_GetAppDir(void);
bool Platform_DirExists(const char *dirPath);
int Platform_MakeDir(const char *dirPath);
double Platform_GetTime(void);

#endif