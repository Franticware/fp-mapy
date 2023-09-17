#include "anglehints.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_egl.h>
#include <cstdio>
#include <cstring>

#define EGL_PLATFORM_ANGLE_ANGLE                       0x3202
#define EGL_PLATFORM_ANGLE_TYPE_ANGLE                  0x3203
#define EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE     0x3204
#define EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE     0x3205
#define EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE             0x3207
#define EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE            0x3208
#define EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE           0x3450
#define EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE           0x320D
#define EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE           0x3209
#define EGL_PLATFORM_ANGLE_DEVICE_TYPE_WARP_ANGLE      0x320B
#define EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE 0x320F

#define EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER 0x320B

void setAngleHints(const char* platform)
{
    static const EGLint d3d11DisplayAttributes[] = {
                    EGL_PLATFORM_ANGLE_TYPE_ANGLE,
                    EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
                    EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER,
                    EGL_TRUE,
                    EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE,
                    EGL_TRUE,
                    EGL_NONE,
                };

    static const EGLint openglDisplayAttributes[] = {
                    EGL_PLATFORM_ANGLE_TYPE_ANGLE,
                    EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE,
                    EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER,
                    EGL_TRUE,
                    EGL_NONE
                };

    static const EGLint d3d9DisplayAttributes[] = {
                    EGL_PLATFORM_ANGLE_TYPE_ANGLE,
                    EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE,
                    EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER,
                    EGL_TRUE,
                    EGL_NONE
                };

    static const EGLint vulkanDisplayAttributes[] = {
                    EGL_PLATFORM_ANGLE_TYPE_ANGLE,
                    EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE,
                    EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE,
                    EGL_TRUE,
                    EGL_NONE
                };

    const EGLint* displayAttributes = nullptr;

    if (strcmp(platform, "d3d11") == 0)
    {
        displayAttributes = d3d11DisplayAttributes;
    }
    else if (strcmp(platform, "d3d9") == 0)
    {
        displayAttributes = d3d9DisplayAttributes;
    }
    else if (strcmp(platform, "opengl") == 0)
    {
        displayAttributes = openglDisplayAttributes;
    }
    else if (strcmp(platform, "vulkan") == 0)
    {
        displayAttributes = vulkanDisplayAttributes;
    }

    if (displayAttributes)
    {
        for (int i = 0; ; ++i)
        {
            char hintNameBuf[64] = { 0 };
            snprintf(hintNameBuf, 63, "SDL_PLATFORM_DISPLAY_ATTRIB%d", i);
            char hintValueBuf[16] = {0};
            snprintf(hintValueBuf, 15, "%d", displayAttributes[i]);
            SDL_SetHint(hintNameBuf, hintValueBuf);
            if (displayAttributes[i] == EGL_NONE)
            {
                break;
            }
        }
    }
}
