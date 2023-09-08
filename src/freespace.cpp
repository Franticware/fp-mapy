#include "freespace.h"

#ifndef __WIN32__

#include <sys/statvfs.h>

long getAvailableSpace(const char* path)
{
    struct statvfs stat;
    if (statvfs(path, &stat) != 0)
    {
        return -1;
    }
    return stat.f_bsize * stat.f_bavail;
}

#else

#include <cstdint>
#include <cstdio>

#include <windows.h>
#include <tchar.h>

long getAvailableSpace(const char* path)
{
    TCHAR buff[1024] = {0};
    MultiByteToWideChar(CP_UTF8, 0, path, strlen(path), buff, 1023);
    ULARGE_INTEGER ret;
    ret.QuadPart = 0;
    GetDiskFreeSpaceEx(buff, &ret, 0, 0);
    return (long)ret.QuadPart;
}

#endif
