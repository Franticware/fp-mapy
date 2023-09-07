#include "load_texture.h"

#include "checkgl.h"

static const GLint textureWrap = GL_REPEAT;
static const GLint textureFilter = GL_LINEAR;
//static const GLint textureFilter = GL_NEAREST;

GLuint load_texture(const Pict& pict)
{
    GLuint tex = 0;
    glGenTextures(1, &tex); checkGL(__FILE__, __LINE__);
    glBindTexture(GL_TEXTURE_2D, tex); checkGL(__FILE__, __LINE__);
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pict.w(), pict.h(), 0, GL_RGB, GL_UNSIGNED_BYTE, pict.c_px()); checkGL(__FILE__, __LINE__);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureFilter); checkGL(__FILE__, __LINE__);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureFilter); checkGL(__FILE__, __LINE__);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrap); checkGL(__FILE__, __LINE__);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrap); checkGL(__FILE__, __LINE__);
    }
    glBindTexture(GL_TEXTURE_2D, 0); checkGL(__FILE__, __LINE__);
    return tex;
}

GLuint load_texture_cube_map(const Pict& pict)
{
    if (pict.w()*6 != pict.h())
        return 0;
    GLuint ret = 0;
    glGenTextures(1, &ret); checkGL(__FILE__, __LINE__);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ret); checkGL(__FILE__, __LINE__);
    static const GLenum faceTarget[6] = {
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
        };
    for (unsigned int i = 0; i != 6; ++i)
    {
        glTexImage2D(faceTarget[i], 0, GL_RGB, pict.w(), pict.w(), 0, GL_RGB, GL_UNSIGNED_BYTE, pict.c_px()+pict.w()*pict.w()*3*i); checkGL(__FILE__, __LINE__); // pixel data
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, textureFilter); checkGL(__FILE__, __LINE__);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, textureFilter); checkGL(__FILE__, __LINE__);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); checkGL(__FILE__, __LINE__);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); checkGL(__FILE__, __LINE__);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0); checkGL(__FILE__, __LINE__);
    return ret;
}

void load_texture_6(GLuint tex[6], const Pict& pict)
{
    if (pict.w()*6 != pict.h())
        return;
    glGenTextures(6, tex); checkGL(__FILE__, __LINE__);
    for (unsigned int i = 0; i != 6; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, tex[i]); checkGL(__FILE__, __LINE__);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pict.w(), pict.w(), 0, GL_RGB, GL_UNSIGNED_BYTE, pict.c_px()+pict.w()*pict.w()*3*i); checkGL(__FILE__, __LINE__); // pixel data
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureFilter); checkGL(__FILE__, __LINE__);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureFilter); checkGL(__FILE__, __LINE__);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); checkGL(__FILE__, __LINE__);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); checkGL(__FILE__, __LINE__);
    }
    glBindTexture(GL_TEXTURE_2D, 0); checkGL(__FILE__, __LINE__);
}
