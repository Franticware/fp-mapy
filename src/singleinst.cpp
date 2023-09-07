#include "singleinst.h"

#ifndef __WIN32__

#include "config.h"

#include <cstdio>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

static int singleInstAux(void)
{
    const int ok = 0;
    const int err = 1;

    struct flock fl;

    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 1;

    int fdlock = 0;

    if ((fdlock = open((gConf.configPath() + "app.lock").c_str(), O_WRONLY|O_CREAT, 0666)) == -1)
        return err;

    if (fcntl(fdlock, F_SETLK, &fl) == -1)
        return err;

    return ok;
}

int singleInst(void)
{
    const int ok = 0;
    int result = singleInstAux();
    if (result != ok)
    {
        fprintf(stderr, "Already running, exiting...\n");
    }
    return result;
}

#else

#include <windows.h>
#include <tchar.h>

int singleInst(void)
{
    const int ok = 0;
    const int err = 1;
    CreateMutex(0, 1, "Global\\fpmapy");
    if (GetLastError() != ERROR_ALREADY_EXISTS)
    {
        return ok;
    }
    MessageBox(NULL, _T("Already running, will exit..."), _T("Error"), MB_ICONERROR | MB_OK);
    return err;
}

#endif
