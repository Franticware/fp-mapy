#include "meshes.h"
#include "skyboxdata.h"
#include "pict.h"
#include "load_texture.h"

#include "checkgl.h"

MeshSkybox::MeshSkybox(bool compat) : m_compat(compat)
{
    if (m_compat)
    {
        generateCompat();
    }
    else
    {
        generate();
    }
}

void MeshSkybox::render(ShaderMng& shaderMng) const
{
    if (m_compat)
    {
        renderPrivCompat(shaderMng);
    }
    else
    {
        renderPriv(shaderMng);
    }
}

void MeshSkybox::generate()
{
    Pict pict;
    pict.loadjpeg(skyboxdata, skyboxdata_len);
    m_skyboxTex[0] = load_texture_cube_map(pict);

    glGenBuffers(1, &m_skyboxVbo); checkGL(__FILE__, __LINE__);
    glBindBuffer(GL_ARRAY_BUFFER, m_skyboxVbo); checkGL(__FILE__, __LINE__);
    static const float skyboxData[12 * 3 * 3] = {
        -1, -1, 1,
        1, 1, 1,
        1, -1, 1,
        1, 1, 1,
        -1, 1, -1,
        1, 1, -1,
        -1, 1, 1,
        -1, -1, -1,
        -1, 1, -1,
        1, -1, -1,
        -1, 1, -1,
        -1, -1, -1,
        1, -1, 1,
        1, 1, -1,
        1, -1, -1,
        -1, -1, 1,
        1, -1, -1,
        -1, -1, -1,
        -1, -1, 1,
        -1, 1, 1,
        1, 1, 1,
        1, 1, 1,
        -1, 1, 1,
        -1, 1, -1,
        -1, 1, 1,
        -1, -1, 1,
        -1, -1, -1,
        1, -1, -1,
        1, 1, -1,
        -1, 1, -1,
        1, -1, 1,
        1, 1, 1,
        1, 1, -1,
        -1, -1, 1,
        1, -1, 1,
        1, -1, -1,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxData), skyboxData, GL_STATIC_DRAW); checkGL(__FILE__, __LINE__);
    glBindBuffer(GL_ARRAY_BUFFER, 0); checkGL(__FILE__, __LINE__);
}

void MeshSkybox::renderPriv(ShaderMng& shaderMng) const
{
    shaderMng.use(ShaderId::Sky);
    glDepthRangef(1, 1); checkGL(__FILE__, __LINE__);
    glActiveTexture(GL_TEXTURE0 + (int)ShaderUniTex::Cube); checkGL(__FILE__, __LINE__);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxTex[0]); checkGL(__FILE__, __LINE__);
    glBindBuffer(GL_ARRAY_BUFFER, m_skyboxVbo); checkGL(__FILE__, __LINE__);
    glEnableVertexAttribArray((GLuint)ShaderAttrib::Pos); checkGL(__FILE__, __LINE__);
    glVertexAttribPointer((GLuint)ShaderAttrib::Pos, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, 0); checkGL(__FILE__, __LINE__);
    glDrawArrays(GL_TRIANGLES, 0, 36); checkGL(__FILE__, __LINE__);
    glDisableVertexAttribArray((GLuint)ShaderAttrib::Pos); checkGL(__FILE__, __LINE__);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0); checkGL(__FILE__, __LINE__);
    glActiveTexture(GL_TEXTURE0); checkGL(__FILE__, __LINE__);
    glDepthRangef(0, 1); checkGL(__FILE__, __LINE__);
}

void MeshSkybox::generateCompat()
{
    Pict pict;
    pict.loadjpeg(skyboxdata, skyboxdata_len);

    load_texture_6(m_skyboxTex, pict);

    glGenBuffers(1, &m_skyboxVbo); checkGL(__FILE__, __LINE__);
    glBindBuffer(GL_ARRAY_BUFFER, m_skyboxVbo); checkGL(__FILE__, __LINE__);
    static const float skyboxData[12 * 3 * 5] = {
        1, -1, -1, 1, 0,
        -1, 1, -1, 0, 1,
        -1, -1, -1, 0, 0,
        1, -1, -1, 1, 0,
        1, 1, -1, 1, 1,
        -1, 1, -1, 0, 1,

        -1, -1, 1, 1, 0,
        1, 1, 1, 0, 1,
        1, -1, 1, 0, 0,
        -1, -1, 1, 1, 0,
        -1, 1, 1, 1, 1,
        1, 1, 1, 0, 1,

        -1, -1, 1, 1, 0,
        1, -1, -1, 0, 1,
        -1, -1, -1, 0, 0,
        -1, -1, 1, 1, 0,
        1, -1, 1, 1, 1,
        1, -1, -1, 0, 1,

        1, 1, 1, 1, 0,
        -1, 1, -1, 0, 1,
        1, 1, -1, 0, 0,
        1, 1, 1, 1, 0,
        -1, 1, 1, 1, 1,
        -1, 1, -1, 0, 1,

        1, -1, 1, 1, 0,
        1, 1, -1, 0, 1,
        1, -1, -1, 0, 0,
        1, -1, 1, 1, 0,
        1, 1, 1, 1, 1,
        1, 1, -1, 0, 1,

        -1, 1, 1, 0, 1,
        -1, -1, -1, 1, 0,
        -1, 1, -1, 1, 1,
        -1, 1, 1, 0, 1,
        -1, -1, 1, 0, 0,
        -1, -1, -1, 1, 0,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxData), skyboxData, GL_STATIC_DRAW); checkGL(__FILE__, __LINE__);
    glBindBuffer(GL_ARRAY_BUFFER, 0); checkGL(__FILE__, __LINE__);
}

void MeshSkybox::renderPrivCompat(ShaderMng& shaderMng) const
{
    shaderMng.use(ShaderId::SkyCompat);
    glDepthRangef(1, 1); checkGL(__FILE__, __LINE__);

    glBindBuffer(GL_ARRAY_BUFFER, m_skyboxVbo); checkGL(__FILE__, __LINE__);
    glEnableVertexAttribArray((GLuint)ShaderAttrib::Pos); checkGL(__FILE__, __LINE__);
    glEnableVertexAttribArray((GLuint)ShaderAttrib::Tex); checkGL(__FILE__, __LINE__);

    glVertexAttribPointer((GLuint)ShaderAttrib::Pos, 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, 0); checkGL(__FILE__, __LINE__);
    glVertexAttribPointer((GLuint)ShaderAttrib::Tex, 2, GL_FLOAT, GL_FALSE, sizeof(float)*5, (void*)(sizeof(float)*3)); checkGL(__FILE__, __LINE__);

    for (size_t i = 0; i != 6; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, m_skyboxTex[i]); checkGL(__FILE__, __LINE__);
        glDrawArrays(GL_TRIANGLES, i * 6, 6); checkGL(__FILE__, __LINE__);
    }
    glDisableVertexAttribArray((GLuint)ShaderAttrib::Pos); checkGL(__FILE__, __LINE__);
    glDisableVertexAttribArray((GLuint)ShaderAttrib::Tex); checkGL(__FILE__, __LINE__);
    glDepthRangef(0, 1); checkGL(__FILE__, __LINE__);
    glBindTexture(GL_TEXTURE_2D, 0); checkGL(__FILE__, __LINE__);
}

MeshFader::MeshFader()
{
    m_fadeOrthoMat = glm::ortho(-1.f, 1.f, -1.f, 1.f);
    glGenBuffers(1, &m_fadeVbo); checkGL(__FILE__, __LINE__);
    glBindBuffer(GL_ARRAY_BUFFER, m_fadeVbo); checkGL(__FILE__, __LINE__);
    static const float fadeData[3 * 3] = {
        -1.5, -4, 0,
        4, 1.5, 0,
        -1.5, 1.5, 0
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(fadeData), fadeData, GL_STATIC_DRAW); checkGL(__FILE__, __LINE__);
    glBindBuffer(GL_ARRAY_BUFFER, 0); checkGL(__FILE__, __LINE__);
}

void MeshFader::render(ShaderMng& shaderMng, float alpha) const
{
    shaderMng.set(ShaderUniMat4::ProjMat, m_fadeOrthoMat);
    shaderMng.set(ShaderUniMat4::ModelViewMat, glm::mat4(1.f));
    shaderMng.set(ShaderUniVec4::Color, glm::vec4(0, 0, 0, alpha));
    shaderMng.use(ShaderId::SingleColor);

    glDisable(GL_DEPTH_TEST); checkGL(__FILE__, __LINE__);
    glEnable(GL_BLEND); checkGL(__FILE__, __LINE__);

    glBindBuffer(GL_ARRAY_BUFFER, m_fadeVbo); checkGL(__FILE__, __LINE__);
    glEnableVertexAttribArray((GLuint)ShaderAttrib::Pos); checkGL(__FILE__, __LINE__);
    glVertexAttribPointer((GLuint)ShaderAttrib::Pos, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, 0); checkGL(__FILE__, __LINE__);
    glDrawArrays(GL_TRIANGLES, 0, 3); checkGL(__FILE__, __LINE__);
    glDisableVertexAttribArray((GLuint)ShaderAttrib::Pos); checkGL(__FILE__, __LINE__);

    glDisable(GL_BLEND); checkGL(__FILE__, __LINE__);
    glEnable(GL_DEPTH_TEST); checkGL(__FILE__, __LINE__);
}
