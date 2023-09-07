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

#include <windows.h>

long getAvailableSpace(const char* path)
{
}

#endif
