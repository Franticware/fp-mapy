#include "meshes.h"
#include "skyboxdata.h"
#include "pict.h"
#include "load_texture.h"

MeshSkybox::MeshSkybox()
{
    Pict pict;
    pict.loadjpeg(skyboxdata, skyboxdata_len);
    m_skyboxTex = load_texture_cube_map(pict);

    glGenBuffers(1, &m_skyboxVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_skyboxVbo);
    static const float skyboxData[12 * 3 * 3] = {
        -1.000000, -1.000000, 1.000000,
        1.000000, 1.000000, 1.000000,
        1.000000, -1.000000, 1.000000,
        1.000000, 1.000000, 1.000000,
        -1.000000, 1.000000, -1.000000,
        1.000000, 1.000000, -1.000000,
        -1.000000, 1.000000, 1.000000,
        -1.000000, -1.000000, -1.000000,
        -1.000000, 1.000000, -1.000000,
        1.000000, -1.000000, -1.000000,
        -1.000000, 1.000000, -1.000000,
        -1.000000, -1.000000, -1.000000,
        1.000000, -1.000000, 1.000000,
        1.000000, 1.000000, -1.000000,
        1.000000, -1.000000, -1.000000,
        -1.000000, -1.000000, 1.000000,
        1.000000, -1.000000, -1.000000,
        -1.000000, -1.000000, -1.000000,
        -1.000000, -1.000000, 1.000000,
        -1.000000, 1.000000, 1.000000,
        1.000000, 1.000000, 1.000000,
        1.000000, 1.000000, 1.000000,
        -1.000000, 1.000000, 1.000000,
        -1.000000, 1.000000, -1.000000,
        -1.000000, 1.000000, 1.000000,
        -1.000000, -1.000000, 1.000000,
        -1.000000, -1.000000, -1.000000,
        1.000000, -1.000000, -1.000000,
        1.000000, 1.000000, -1.000000,
        -1.000000, 1.000000, -1.000000,
        1.000000, -1.000000, 1.000000,
        1.000000, 1.000000, 1.000000,
        1.000000, 1.000000, -1.000000,
        -1.000000, -1.000000, 1.000000,
        1.000000, -1.000000, 1.000000,
        1.000000, -1.000000, -1.000000,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxData), skyboxData, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MeshSkybox::render(ShaderMng& shaderMng) const
{
    shaderMng.use(ShaderId::Sky);
    glDepthRangef(1, 1);
    glActiveTexture(GL_TEXTURE0 + (int)ShaderUniTex::Cube);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_skyboxTex);
    glBindBuffer(GL_ARRAY_BUFFER, m_skyboxVbo);
    glEnableVertexAttribArray((GLuint)ShaderAttrib::Pos);
    glVertexAttribPointer((GLuint)ShaderAttrib::Pos, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, 0);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDisableVertexAttribArray((GLuint)ShaderAttrib::Pos);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glActiveTexture(GL_TEXTURE0);
    glDepthRangef(0, 1);
}

MeshFader::MeshFader()
{
    m_fadeOrthoMat = glm::ortho(-1.f, 1.f, -1.f, 1.f);
    glGenBuffers(1, &m_fadeVbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_fadeVbo);
    static const float fadeData[3 * 3] = {
        -1.5, -4, 0,
        4, 1.5, 0,
        -1.5, 1.5, 0
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(fadeData), fadeData, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MeshFader::render(ShaderMng& shaderMng, float alpha) const
{
    shaderMng.set(ShaderUniMat4::ProjMat, m_fadeOrthoMat);
    shaderMng.set(ShaderUniMat4::ModelViewMat, glm::mat4(1.f));
    shaderMng.set(ShaderUniVec4::Color, glm::vec4(0, 0, 0, alpha));
    shaderMng.use(ShaderId::SingleColor);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    glBindBuffer(GL_ARRAY_BUFFER, m_fadeVbo);
    glEnableVertexAttribArray((GLuint)ShaderAttrib::Pos);
    glVertexAttribPointer((GLuint)ShaderAttrib::Pos, 3, GL_FLOAT, GL_FALSE, sizeof(float)*3, 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray((GLuint)ShaderAttrib::Pos);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}
