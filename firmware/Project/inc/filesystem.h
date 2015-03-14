#ifndef _FILE_SYSTEM_H__
#define _FILE_SYSTEM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

void FileSystem_Init(void);
bool FileSystem_MkFs(void);
bool FileSystem_MkDir(const char *dirname);

#ifdef __cplusplus
};
#endif

#endif