#include "maptile.h"

#include <cstdio>

#include <GLES2/gl2.h>

#include "shadermng.h"
#include "vts/meshgz.h"
#include "downl.h"
#include "load_texture.h"
#include "culling.h"
#include "config.h"
#include "freespace.h"
#include "checkgl.h"

MapTile::MapTile()
{
}

static void readFile(FILE* fin, std::vector<uint8_t>& data)
{
    data.clear();
    fseek(fin, 0, SEEK_END);
    long sz = ftell(fin);
    rewind(fin);
    data.resize(sz);
    if (fread(data.data(), 1, data.size(), fin) != data.size())
    {
        data.clear();
    }
}

static void writeFile(const char* fileName, const std::vector<uint8_t>& data)
{
    FILE* fout = fopen(fileName, "wb");
    if (fout)
    {
        fwrite(data.data(), 1, data.size(), fout);
        fclose(fout);
    }
}

static void emptyFile(const char* fileName)
{
    FILE* fout = fopen(fileName, "wb");
    if (fout)
    {
        fclose(fout);
    }
}

static void emptyFile(const char* basePath, const char* fileName)
{
    char buff[1024] = {0};
    snprintf(buff, 1023, "%s%s", basePath, fileName);
    emptyFile(buff);
}

static int downlGetCached(const char* baseUrl, const char* basePath, const char* fileName, bool cacheEnabled, std::vector<uint8_t>& data)
{
    const int ok = 0;
    int result = ok;
    char buff[1024] = {0};
    FILE* fin = nullptr;
    if (cacheEnabled)
    {
        snprintf(buff, 1023, "%s%s", basePath, fileName);
        fin = fopen(buff, "rb");
    }
    data.clear();
    if (fin)
    {
        readFile(fin, data);
        fclose(fin);
    }
    if (data.empty())
    {
        snprintf(buff, 1023, "%s%s", baseUrl, fileName);
        result = downlGet(buff, data);
        if (cacheEnabled)
        {
            if (getAvailableSpace(basePath) > long(gConf.cacheMinFreeSpaceGB()) * long(1024) * long(1024) * long(1024))
            {
                snprintf(buff, 1023, "%s%s", basePath, fileName);
                writeFile(buff, data);
            }
        }
    }
    return result;
}

int MapTile::load(MapTileKey k)
{
    const int ok = 0;
    //const int err = 1;
    //const int err404 = 404;
    const char* baseUrl = gConf.baseUrlCstr();
    const char* cachePath = gConf.cachePathCstr();
    bool cacheEnabled = gConf.cacheEnabled();
    //const char* baseUrl = "http://mapserver-3d.mapy.cz/latestStage/tilesets/cities/";
    char fileName[64] = {0};
    snprintf(fileName, 63, "21-%06d-%06d.bin", k.x, k.y);
    int res = 0;
    std::vector<uint8_t> data;
    if ((downlGetCached(baseUrl, cachePath, fileName, cacheEnabled, data)) != ok)
    {
        fprintf(stderr, "%d: %s%s\n", res, baseUrl, fileName);
        fflush(stderr);
        return res;
    }
    if ((res = loadMeshZ(data, m_mesh)) != ok)
    {
        if (cacheEnabled)
        {
            emptyFile(cachePath, fileName);
        }
        return res;
    }
    m_pics.resize(m_mesh.submeshes.size());
    for (size_t i = 0; i != m_mesh.submeshes.size(); ++i)
    {
        snprintf(fileName, 63, "21-%06d-%06d-%d.jpg", k.x, k.y, int(i));
        if ((downlGetCached(baseUrl, cachePath, fileName, cacheEnabled, data)) != ok)
        {
            fprintf(stderr, "%d: %s%s\n", res, baseUrl, fileName);
            fflush(stderr);
            return res;
        }
        if ((res = m_pics[i].loadjpeg(data.data(), data.size())) != ok)
        {
            if (cacheEnabled)
            {
                emptyFile(cachePath, fileName);
            }
            return res;
        }
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
        glDeleteTextures(1, &(m_submeshes[i].m_tex)); checkGL(__FILE__, __LINE__);
        glDeleteBuffers(1, &(m_submeshes[i].m_vbo)); checkGL(__FILE__, __LINE__);
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
        glGenBuffers(1, &subm.m_vbo); checkGL(__FILE__, __LINE__);
        glBindBuffer(GL_ARRAY_BUFFER, subm.m_vbo); checkGL(__FILE__, __LINE__);
        glBufferData(GL_ARRAY_BUFFER, mt.m_vboData[i].size() * sizeof(float), mt.m_vboData[i].data(), GL_STATIC_DRAW); checkGL(__FILE__, __LINE__);
        glBindBuffer(GL_ARRAY_BUFFER, 0); checkGL(__FILE__, __LINE__);
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

void TileGl::render() const
{
    for (const auto& subm : m_submeshes)
    {
        glBindTexture(GL_TEXTURE_2D, subm.m_tex); checkGL(__FILE__, __LINE__);
        glBindBuffer(GL_ARRAY_BUFFER, subm.m_vbo); checkGL(__FILE__, __LINE__);
        glEnableVertexAttribArray((GLuint)ShaderAttrib::Pos); checkGL(__FILE__, __LINE__);
        glEnableVertexAttribArray((GLuint)ShaderAttrib::Tex); checkGL(__FILE__, __LINE__);
        glVertexAttribPointer((GLuint)ShaderAttrib::Pos, 3, GL_FLOAT, GL_FALSE, sizeof(float)*5, 0); checkGL(__FILE__, __LINE__);
        glVertexAttribPointer((GLuint)ShaderAttrib::Tex, 2, GL_FLOAT, GL_FALSE, sizeof(float)*5, (void*)(sizeof(float)*3)); checkGL(__FILE__, __LINE__);
        glDrawArrays(GL_TRIANGLES, 0, subm.m_vertCount); checkGL(__FILE__, __LINE__);
        glDisableVertexAttribArray((GLuint)ShaderAttrib::Pos); checkGL(__FILE__, __LINE__);
        glDisableVertexAttribArray((GLuint)ShaderAttrib::Tex); checkGL(__FILE__, __LINE__);
    }
}
