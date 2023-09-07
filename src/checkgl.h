#ifndef CHECKGL_H
#define CHECKGL_H

#include <GLES2/gl2.h>
#include <cstdio>

inline void checkGL(const char* file, int line)
{
    (void)file; (void)line;
#if 0
    GLenum err = glGetError();
    switch (err)
    {
    case GL_NO_ERROR:
        return;
    case GL_INVALID_ENUM:
        fprintf(stderr, "\nglGetError:%s:%d: GL_INVALID_ENUM\n", file, line);
        break;
    case GL_INVALID_VALUE:
        fprintf(stderr, "\nglGetError:%s:%d: GL_INVALID_ENUM\n", file, line);
        break;
    case GL_INVALID_OPERATION:
        fprintf(stderr, "\nglGetError:%s:%d: GL_INVALID_ENUM\n", file, line);
        break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        fprintf(stderr, "\nglGetError:%s:%d: GL_INVALID_ENUM\n", file, line);
        break;
    case GL_OUT_OF_MEMORY:
        fprintf(stderr, "\nglGetError:%s:%d: GL_INVALID_ENUM\n", file, line);
        break;
    default:
        fprintf(stderr, "\nglGetError:%s:%d: %u\n", file, line, err);
        break;
    }
    fflush(stderr);
#endif
}

#endif // CHECKGL_H
