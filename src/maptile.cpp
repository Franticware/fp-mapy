#include "maptile.h"

#include <cstdio>

#include <GLES2/gl2.h>

#include "shadermng.h"
#include "vts/meshgz.h"
#include "downl.h"
#include "load_texture.h"
#include "culling.h"
#include "config.h"

MapTile::MapTile()
{
}

int MapTile::load(MapTileKey k)
{
    const int ok = 0;
    //const int err = 1;
    //const int err404 = 404;
    const char* baseUrl = gConf.baseUrl().c_str();
    char urlBuff[256] = {0};
    snprintf(urlBuff, 255, "%s21-%06d-%06d.bin", baseUrl, k.x, k.y);
    int res = 0;
    std::vector<uint8_t> data;
    if ((res = downlGet(urlBuff, data)) != ok)
    {
        printf("%d: %s\n", res, urlBuff);
        fflush(stdout);
        return res;
    }
    if ((res = loadMeshZ(data, m_mesh)) != ok)
    {
        return res;
    }
    m_pics.resize(m_mesh.submeshes.size());
    for (size_t i = 0; i != m_mesh.submeshes.size(); ++i)
    {
        snprintf(urlBuff, 255, "%s21-%06d-%06d-%d.jpg", baseUrl, k.x, k.y, int(i));
        if ((res = downlGet(urlBuff, data)) != ok)
        {
            printf("%d: %s\n", res, urlBuff);
            fflush(stdout);
            return res;
        }
        m_pics[i].loadjpeg(data.data(), data.size());
    }
    return ok;
}

void MapTile::calcPosRot(glm::dvec3& pos, glm::mat3& rot) const
{
    double minExtent[3] = {0.0, 0.0, 0.0};
    double maxExtent[3] = {0.0, 0.0, 0.0};
    for (size_t h = 0; h != m_mesh.submeshes.size(); ++h)
    {
        for (size_t i = 0; i != m_mesh.submeshes[h].vertices.size(); ++i)
        {
            auto vert = m_mesh.submeshes[h].vertices[i];
            if (i == 0 && h == 0)
            {
                for (int j = 0; j != 3; ++j)
                {
                    minExtent[j] = maxExtent[j] = vert[j];
                }
            }
            else
            {
                for (int j = 0; j != 3; ++j)
                {
                    minExtent[j] = std::min(double(minExtent[j]), (vert[j]));
                    maxExtent[j] = std::max(double(maxExtent[j]), (vert[j]));
                }
            }
        }
    }
    pos.x = (minExtent[0] + maxExtent[0]) * 0.5;
    pos.y = (minExtent[1] + maxExtent[1]) * 0.5;
    pos.z = (minExtent[2] + maxExtent[2]) * 0.5;
    glm::vec3 dirY = pos;
    dirY = glm::normalize(dirY);
    glm::vec3 dirX(1.f, 0.f, 0.f);
    glm::vec3 dirZ = glm::cross(dirX, dirY);
    dirX = glm::cross(dirZ, dirY);
    dirX = glm::normalize(dirX);
    dirZ = glm::normalize(dirZ);
    rot = glm::mat3(dirX, dirY, dirZ);
    rot = glm::transpose(rot);
}

void MapTile::precomp(const glm::dvec3& pos, const glm::mat3& rot)
{
    m_vboData.clear();
    m_vboData.resize(m_mesh.submeshes.size());
    m_mid = glm::vec3(0.f, 0.f, 0.f);
    bool first = true;
    glm::vec3 vertMin(0.f, 0.f, 0.f);
    glm::vec3 vertMax(0.f, 0.f, 0.f);

    for (size_t h = 0; h != m_mesh.submeshes.size(); ++h)
    {
        for (size_t i = 0; i != m_mesh.submeshes[h].faces.size(); ++i)
        {
            for (size_t j = 0; j != 3; ++j)
            {
                size_t vi = m_mesh.submeshes[h].faces[i][j];
                size_t ti = m_mesh.submeshes[h].facesTc[i][j];
                glm::vec3 offsetVert;
                for (size_t k = 0; k != 3; ++k)
                {
                    offsetVert[k] = m_mesh.submeshes[h].vertices[vi][k] - pos[k];
                }
                offsetVert = rot * offsetVert * glm::vec3(1.f, 1.f, -1.f);
                if (first)
                {
                    vertMin = offsetVert;
                    vertMax = offsetVert;
                    first = false;
                }
                else
                {
                    vertMin.x = std::min(vertMin.x, offsetVert.x);
                    vertMin.y = std::min(vertMin.y, offsetVert.y);
                    vertMin.z = std::min(vertMin.z, offsetVert.z);
                    vertMax.x = std::max(vertMax.x, offsetVert.x);
                    vertMax.y = std::max(vertMax.y, offsetVert.y);
                    vertMax.z = std::max(vertMax.z, offsetVert.z);
                }
                for (size_t k1 = 0; k1 != 3; ++k1)
                {
                    size_t k = k1;
                    m_vboData[h].push_back(offsetVert[k]);
                }
                for (size_t k = 0; k != 2; ++k)
                {
                    m_vboData[h].push_back(m_mesh.submeshes[h].tc[ti][k]);
                }
            }
        }
    }
    m_mid = (vertMin + vertMax) * 0.5f;
    m_mid = glm::vec3(m_mid.x, m_mid.y, m_mid.z);
    m_bbox = (vertMax - vertMin) * 0.5f;
}

TileGl::TileGl()
{
}

TileGl::~TileGl()
{
    for (size_t i = 0; i != m_submeshes.size(); ++i)
    {
        glDeleteTextures(1, &(m_submeshes[i].m_tex));
        glDeleteBuffers(1, &(m_submeshes[i].m_vbo));
    }
    m_submeshes.clear();
}

void TileGl::load(const MapTile& mt)
{
    if (!m_submeshes.empty())
    {
        // unexpected
        return;
    }
    if (mt.m_pics.size() != mt.m_vboData.size())
    {
        // unexpected
        return;
    }
    m_mid = mt.m_mid;
    m_bbox = mt.m_bbox;
    for (size_t i = 0; i != mt.m_vboData.size(); ++i)
    {
        SubMesh subm;
        glGenBuffers(1, &subm.m_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, subm.m_vbo);
        glBufferData(GL_ARRAY_BUFFER, mt.m_vboData[i].size() * sizeof(float), mt.m_vboData[i].data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        subm.m_vertCount = mt.m_vboData[i].size() / 5;
        subm.m_tex = load_texture(mt.m_pics[i]);
        m_submeshes.push_back(subm);
    }
}

float TileGl::dist(const glm::vec3& pos) const
{
    return std::abs(m_mid.x - pos.x) + std::abs(m_mid.y - pos.y) + std::abs(m_mid.z - pos.z);
}

bool TileGl::cullFrustum(const glm::mat4& m) const
{
    return cullingFrustumBox(m, m_mid, m_bbox);
}

inline void checkGL() { }

void TileGl::render() const
{
    for (const auto& subm : m_submeshes)
    {
        glBindTexture(GL_TEXTURE_2D, subm.m_tex);
        glBindBuffer(GL_ARRAY_BUFFER, subm.m_vbo);
        glEnableVertexAttribArray((GLuint)ShaderAttrib::Pos); checkGL();
        glEnableVertexAttribArray((GLuint)ShaderAttrib::Tex); checkGL();
        glVertexAttribPointer((GLuint)ShaderAttrib::Pos, 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, 0);
        glVertexAttribPointer((GLuint)ShaderAttrib::Tex, 2, GL_FLOAT, GL_FALSE, sizeof(float)*5, (void*)(sizeof(float)*3));
        glDrawArrays(GL_TRIANGLES, 0, subm.m_vertCount);
        glDisableVertexAttribArray((GLuint)ShaderAttrib::Pos); checkGL();
        glDisableVertexAttribArray((GLuint)ShaderAttrib::Tex); checkGL();
    }
}
