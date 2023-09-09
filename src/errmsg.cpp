#include "errmsg.h"
#include <cstdarg>
#include <cstdio>

void errmsgPrintf(const char* format, ...)
{
    char buff[1024] = {0};
    va_list args;
    va_start(args, format);
    vsnprintf(buff, 1023, format, args);
    va_end(args);

    errmsgPuts(buff);
}

#ifndef __WIN32__

void errmsgPuts(const char* message)
{
    fprintf(stderr, "\n%s\n", message);
    fflush(stdout);
}

#else

#include <windows.h>
#include <tchar.h>

void errmsgPuts(const char* message)
{
    TCHAR buff[1024] = {0};
    MultiByteToWideChar(CP_UTF8, 0, message, strlen(message), buff, 1023);
    MessageBox(NULL, buff, _T("Error"), MB_ICONERROR | MB_OK);
}

#endif
