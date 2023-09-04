#ifndef MESHES_H
#define MESHES_H

#include <GLES2/gl2.h>
#include "shadermng.h"

class MeshSkybox
{
public:
    MeshSkybox();
    void render(ShaderMng& shaderMng) const;

private:
    GLuint m_skyboxTex = 0;
    GLuint m_skyboxVbo = 0;
};

class MeshFader
{
public:
    MeshFader();
    void render(ShaderMng& shaderMng, float alpha) const;

private:
    glm::mat4 m_fadeOrthoMat;

    GLuint m_fadeVbo = 0;
};

#endif // MESHES_H
