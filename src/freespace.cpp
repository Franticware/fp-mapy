#include "freespace.h"

#include <sys/statvfs.h>

long getAvailableSpace(const char* path)
{
    // https://www.systutorials.com/how-to-get-available-filesystem-space-on-linux-a-c-function-and-a-cpp-example/
    struct statvfs stat;

    if (statvfs(path, &stat) != 0) {
        // error happens, just quits here
        return -1;
    }

    // the available size is f_bsize * f_bavail
    return stat.f_bsize * stat.f_bavail;
}
