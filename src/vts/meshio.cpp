/**
 * Copyright (c) 2017 Melown Technologies SE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * *  Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <vector>
#include <numeric>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include "binaryio.hpp"

#include "mesh.hpp"

namespace fs = boost::filesystem;
namespace bio = boost::iostreams;
namespace bin = utility::binaryio;

namespace vtslibs { namespace vts { namespace detail {

namespace {
    // mesh proper
    const char MAGIC[2] = { 'M', 'E' };
    const std::uint16_t VERSION = 3;

    struct SubMeshFlag { enum : std::uint8_t {
        internalTexture = 0x1
        , externalTexture = 0x2
        /*, reserved = 0x4 */
        , textureMode = 0x8
    }; };
} // namespace

////////////////////////////////////////////////////////////////////////////////

namespace {

class DeltaReader
{
public:
    DeltaReader(std::istream &in) : in_(in) {}

    unsigned readWord()
    {
        uint8_t byte1, byte2;
        bin::read(in_, byte1);
        if (byte1 & 0x80) {
            bin::read(in_, byte2);
            return (int(byte1) & 0x7f) | (int(byte2) << 7);
        }
        return byte1;
    }

    int readDelta(int &last)
    {
        unsigned word = readWord();
        // FIXME: fix for proper bit operations
        int delta((word >> 1) ^ (-(word & 1)));
        return (last += delta);
    }

private:
    std::istream &in_;
};


void loadSubmeshVersion3(std::istream &in, SubMesh &sm, std::uint8_t flags
                         , const math::Extents3 &bbox)
{
    math::Point3d center = 0.5*(bbox.ll + bbox.ur);
    math::Point3d bbsize(bbox.ur - bbox.ll);
    double scale = std::max(bbsize(0), std::max(bbsize(1), bbsize(2)));

    DeltaReader dr(in);

    // load vertices
    std::uint16_t vertexCount;
    {
        bin::read(in, vertexCount);
        sm.vertices.resize(vertexCount);

        std::uint16_t quant;
        bin::read(in, quant);

        int last[3] = {0, 0, 0};
        double multiplier = 1.0 / quant;
        for (auto& vertex : sm.vertices) {
            for (int i = 0; i < 3; i++)
            {
                int qcoord = dr.readDelta(last[i]);
                double coord = double(qcoord) * multiplier;
                vertex(i) = coord*scale + center(i);
            }
        }
    }

    // load external coords
    if (flags & SubMeshFlag::externalTexture)
    {
        sm.etc.resize(vertexCount);

        std::uint16_t quant;
        bin::read(in, quant);

        int last[2] = {0, 0};
        double multiplier = 1.0 / quant;
        for (auto& etc : sm.etc) {
            for (int i = 0; i < 2; i++)
            {
                int qcoord = dr.readDelta(last[i]);
                etc(i) = double(qcoord) * multiplier;
            }
        }
    }

    // load texcoords
    if (flags & SubMeshFlag::internalTexture)
    {
        std::uint16_t tcCount;
        bin::read(in, tcCount);
        sm.tc.resize(tcCount);

        std::uint16_t tquant[2];
        bin::read(in, tquant[0]);
        bin::read(in, tquant[1]);

        int last[3] = {0, 0, 0};
        double multiplier[2] = {1.0 / tquant[0], 1.0 / tquant[1]};
        for (auto &texc : sm.tc) {
            for (int i = 0; i < 2; i++)
            {
                int qcoord = dr.readDelta(last[i]);
                texc(i) = double(qcoord) * multiplier[i];
            }
        }
    }

    // load faces
    {
        std::uint16_t faceCount;
        bin::read(in, faceCount);
        sm.faces.resize(faceCount);

        int high = 0;
        for (auto &face : sm.faces) {
            for (int i = 0; i < 3; i++)
            {
                int delta = dr.readWord();
                int index = high - delta;
                if (!delta) { high++; }
                face(i) = index;
            }
        }

        if (flags & SubMeshFlag::internalTexture) {
            sm.facesTc.resize(faceCount);

            int high = 0;
            for (auto &face : sm.facesTc) {
                for (int i = 0; i < 3; i++)
                {
                    int delta = dr.readWord();
                    int index = high - delta;
                    if (!delta) { high++; }
                    face(i) = index;
                }
            }
        }
    }
}

void loadSubmeshVersion2(std::istream &in, SubMesh &sm, std::uint8_t flags
                         , const math::Extents3 &bbox)
{
    // helper functions
    auto loadVertexComponent([&in](double o, double s) -> double
    {
        std::uint16_t v;
        bin::read(in, v);
        return o + ((v * s) / std::numeric_limits<std::uint16_t>::max());
    });

    auto loadTexCoord([&in]() -> double
    {
        std::uint16_t v;
        bin::read(in, v);
        return (double(v) / std::numeric_limits<std::uint16_t>::max());
    });

    math::Point3d bbsize(bbox.ur - bbox.ll);

    std::uint16_t vertexCount;
    bin::read(in, vertexCount);
    sm.vertices.resize(vertexCount);

    if (flags & SubMeshFlag::externalTexture) {
        sm.etc.resize(vertexCount);
    }

    // load all vertex components
    auto ietc(sm.etc.begin());
    for (auto &vertex : sm.vertices) {
        vertex(0) = loadVertexComponent(bbox.ll(0), bbsize(0));
        vertex(1) = loadVertexComponent(bbox.ll(1), bbsize(1));
        vertex(2) = loadVertexComponent(bbox.ll(2), bbsize(2));

        if (flags & SubMeshFlag::externalTexture) {
            (*ietc)(0) = loadTexCoord();
            (*ietc)(1) = loadTexCoord();
            ++ietc;
        }
    }

    // load (internal) texture coordinates
    if (flags & SubMeshFlag::internalTexture) {
        std::uint16_t tcCount;
        bin::read(in, tcCount);
        sm.tc.resize(tcCount);
        for (auto &tc : sm.tc) {
            tc(0) = loadTexCoord();
            tc(1) = loadTexCoord();
        }
    }

    // load faces
    std::uint16_t faceCount;
    bin::read(in, faceCount);
    sm.faces.resize(faceCount);

    if (flags & SubMeshFlag::internalTexture) {
        sm.facesTc.resize(faceCount);
    }
    auto ifacesTc(sm.facesTc.begin());

    for (auto &face : sm.faces) {
        std::uint16_t index;
        bin::read(in, index); face(0) = index;
        bin::read(in, index); face(1) = index;
        bin::read(in, index); face(2) = index;

        // load (optional) texture coordinate indices
        if (flags & SubMeshFlag::internalTexture) {
            bin::read(in, index); (*ifacesTc)(0) = index;
            bin::read(in, index); (*ifacesTc)(1) = index;
            bin::read(in, index); (*ifacesTc)(2) = index;
            ++ifacesTc;
        }
    }
}

// get submesh from submesh -> identity
inline SubMesh& getSubmesh(SubMesh& sm) { return sm; }

// get submesh from normalized submesh
inline SubMesh& getSubmesh(NormalizedSubMesh &sm) { return sm.submesh; }

// helpers normalized bbox
const math::Extents3 normBbox(-1.0, -1.0, -1.0, +1.0, +1.0, +1.0);

inline void loadSubmeshVersion2(std::istream &in, NormalizedSubMesh &sm
                                , std::uint8_t flags
                                , const math::Extents3 &bbox)
{
    loadSubmeshVersion2(in, sm.submesh, flags, normBbox);
    sm.extents = bbox;
}

inline void loadSubmeshVersion3(std::istream &in, NormalizedSubMesh &sm
                                , std::uint8_t flags
                                , const math::Extents3 &bbox)
{
    loadSubmeshVersion3(in, sm.submesh, flags, bbox);

    // re-compute extents
    const auto es(math::size(sm.extents));
    const auto center(math::center(sm.extents));

    const math::Point3 scale(2.0 / es.width, 2.0 / es.height, 2.0 / es.depth);

    // normalize
    for (auto &v : sm.submesh.vertices) {
        v(0) = (v(0) - center(0)) * scale(0);
        v(1) = (v(1) - center(1)) * scale(1);
        v(2) = (v(2) - center(2)) * scale(2);
    }
}

template <typename MeshType>
void loadMeshProperImpl(std::istream &in, MeshType &mesh)
{
    // Load mesh headers first
    char magic[sizeof(MAGIC)];
    std::uint16_t version;

    bin::read(in, magic);
    bin::read(in, version);

    if (std::memcmp(magic, MAGIC, sizeof(MAGIC))) {
    }
    if (version > VERSION) {
    }

    // ignore mean undulation
    double reserved;
    bin::read(in, reserved);

    std::uint16_t subMeshCount;
    bin::read(in, subMeshCount);

    // make room for sub-meshes and load them all
    mesh.resize(subMeshCount);
    for (auto &meshItem : mesh) {
        auto &sm(getSubmesh(meshItem));

        std::uint8_t flags;
        bin::read(in, flags);

        if (version >= 2) {
            // submesh surface reference was added in version=2
            std::uint8_t surfaceReference;
            bin::read(in, surfaceReference);
            sm.surfaceReference = surfaceReference;
        }

        // load (external) texture layer information
        std::uint16_t u16;
        bin::read(in, u16);

        if (flags & SubMeshFlag::textureMode) {
            sm.textureMode = SubMesh::TextureMode::external;
            // leave textureLayer undefined if zero
            if (u16) {
                sm.textureLayer = u16;
            }
        }

        // load sub-mesh bounding box
        math::Extents3 bbox;
        bin::read(in, bbox.ll(0));
        bin::read(in, bbox.ll(1));
        bin::read(in, bbox.ll(2));
        bin::read(in, bbox.ur(0));
        bin::read(in, bbox.ur(1));
        bin::read(in, bbox.ur(2));

        if (version >= 3) {
            loadSubmeshVersion3(in, meshItem, flags, bbox);
        } else {
            loadSubmeshVersion2(in, meshItem, flags, bbox);
        }
    }
}

} // namespace

void loadMeshProper(std::istream &in, Mesh &mesh)
{
    loadMeshProperImpl(in, mesh.submeshes);
}

} // namespace detail

NormalizedSubMesh::list
loadMeshProperNormalized(std::istream &in)
{
    NormalizedSubMesh::list submeshes;

    // raw file
    detail::loadMeshProperImpl(in, submeshes);

    return submeshes;
}

} } // namespace vadstena::vts
