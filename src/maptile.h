#ifndef MAPTILE_H
#define MAPTILE_H

#include <vector>
#include <cstdint>
#include <glm/glm.hpp>
#include <GLES2/gl2.h>
#include "vts/mesh.hpp"
#include "pict.h"

#define MTS_EMPTY 0
#define MTS_IN_PROGRESS 1
#define MTS_DONE 2
#define MTS_FAIL 3
#define MTS_404 404

struct MapTileKey
{
    MapTileKey(int xPrm, int yPrm) : x(xPrm), y(yPrm) { }
    bool operator<(const MapTileKey& mtk) const
    {
        if (x < mtk.x) return true;
        if (x > mtk.x) return false;
        if (y < mtk.y) return true;
        return false;
    }
    bool operator==(const MapTileKey& mtk) const
    {
        return x == mtk.x && y == mtk.y;
    }
    bool operator!=(const MapTileKey& mtk) const
    {
        return !(*this == mtk);
    }
    int dist(const MapTileKey& mtk) const
    {
        return std::abs(x - mtk.x) + std::abs(y - mtk.y);
    }
    int x;
    int y;
};

class MapTile
{
public:
    MapTile();
    int load(MapTileKey k, bool& dl);
    void calcPosRot(glm::dvec3& pos, glm::mat3& rot) const;
    void precomp(const glm::dvec3& pos, const glm::mat3& rot);

    friend class TileGl;

private:
    // loaded data
    vtslibs::vts::Mesh m_mesh;
    std::vector<Pict> m_pics;

    // precomputed data
    std::vector<std::vector<float> > m_vboData;
    glm::vec3 m_mid; // midpoint of vertex data
    glm::vec3 m_bbox; // half bbox dimensions
};

class TileGl
{
public:
    TileGl();
    ~TileGl();
    void load(const MapTile& mt);
    void render() const;
    float dist(const glm::vec3& pos) const;
    bool cullFrustum(const glm::mat4& m) const;

private:
    struct SubMesh
    {
        GLuint m_tex = 0;
        GLuint m_vbo = 0;
        size_t m_vertCount = 0;
    };

    std::vector<SubMesh> m_submeshes;
    glm::vec3 m_mid = glm::vec3(0.f, 0.f, 0.f); // midpoint of vertex data
    glm::vec3 m_bbox = glm::vec3(0.f, 0.f, 0.f); // half bbox dimensions
};

#endif // MAPTILE_H
