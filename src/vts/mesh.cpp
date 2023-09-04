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
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/restrict.hpp>

#include "expect.hpp"

#include "geometry.hpp"

#include "mesh.hpp"
#include "math.hpp"

namespace fs = boost::filesystem;
namespace bio = boost::iostreams;

namespace vtslibs { namespace vts {

math::Extents3 extents(const SubMesh &submesh)
{
    return computeExtents(submesh.vertices);
}

math::Extents3 extents(const ConstSubMeshRange &smRange)
{
    math::Extents3 e(math::InvalidExtents{});
    for (const auto &sm : smRange) {
        e = unite(e, extents(sm));
    }
    return e;
}

namespace {

template <typename Vertices>
double faceArea(const Vertices &vertices, const Face &face)
{
    return triangleArea(vertices[face[0]]
                        , vertices[face[1]]
                        , vertices[face[2]]);
}

} // namespace

SubMeshArea area(const math::Points3d &vertices
                 , const Faces &faces
                 , const math::Points2d *tc
                 , const Faces *facesTc
                 , const math::Points2d *etc
                 , const VertexMask *mask)
{
    SubMeshArea a;

    if (faces.empty()) { return a; }

    if (mask) {
        // we have mask and texturing info, process
        utility::expect((vertices.size() == mask->size())
                        , "Submesh vertex list size (%d) different "
                        "from vertex mask size (%d)."
                        , faces.size(), mask->size());
    }

    auto valid([&](const Face &face)
    {
        return ((*mask)[face(0)] && (*mask)[face(1)] && (*mask)[face(2)]);
    });

    if (mask && tc && facesTc) {

        // we have to check 3d face validity before computing
        auto ifacesTc(facesTc->begin());
        for (const auto &face : faces) {
            const auto &faceTc(*ifacesTc++);

            if (!valid(face)) { continue; }

            // valid face, compute both areas
            // mesh
            a.mesh += faceArea(vertices, face);

            // texturing mesh
            a.internalTexture += faceArea(*tc, faceTc);
        }
    } else {
        // other cases
        if (mask) {
            for (const auto &face : faces) {
                if (valid(face)) {
                    a.mesh += faceArea(vertices, face);
                }
            }
        } else {
            for (const auto &face : faces) {
                a.mesh += faceArea(vertices, face);
            }
        }

        // internal texture
        if (tc && facesTc) {
            for (const auto &faceTc : *facesTc) {
                a.internalTexture += faceArea(*tc, faceTc);
            }
        }
    }

    // external texture
    if (etc) {
        for (const auto &face : faces) {
            a.externalTexture += faceArea(*etc, face);
        }
    }

    return a;
}

namespace {

template <typename Container>
const Container* nonempty(const Container &c)
{
    return c.empty() ? nullptr : &c;
}

} // namespace

SubMesh SubMesh::cleanUp() const
{
    SubMesh ret;
    cloneMetadataInto(ret);

    // make room
    ret.vertices.reserve(vertices.size());
    ret.etc.reserve(etc.size());
    ret.tc.reserve(tc.size());
    ret.faces.reserve(faces.size());
    ret.facesTc.reserve(facesTc.size());

    bool hasEtc(!etc.empty());
    bool hasTc(!tc.empty());

    typedef std::vector<int> Index;

    struct Mapping {
        int vertex;
        int chain;
        Mapping(int vertex = -1) : vertex(vertex), chain(-1) {}
        typedef std::vector<Mapping> list;
    };

    Index vindex(vertices.size(), -1);
    Index tindex;
    Mapping::list tcMap;

    if (hasTc) {
        // populate only if we have internal textures
        tindex.assign(tc.size(), -1);
        tcMap.reserve(tc.size());
    }

    // Adds 3D mesh vertex to the output mesh, reused already added one
    const auto &addVertex([&](Face::value_type &vertex)
    {
        int &idx(vindex[vertex]);
        if (idx < 0) {
            idx = int(ret.vertices.size());
            ret.vertices.push_back(vertices[vertex]);
            if (hasEtc) { ret.etc.push_back(etc[vertex]); }
        }
        vertex = idx;
    });

    /** Allocates new tc index in the output mesh.
     */
    const auto &allocateTcIndex([&](Face::value_type &tcVertex, int vertex)
    {
        int idx(ret.tc.size());
        ret.tc.push_back(tc[tcVertex]);
        tcMap.emplace_back(vertex);
        return idx;
    });

    /** Adds 2D texturing coordinates to the output mesh, resuses already added
     *  one. Clones texturing coordinates if more than one 3D vertex maps to
     *  tc.
     */
    const auto &addTc([&](Face::value_type &tcVertex, int vertex)
    {
        int &idx(tindex[tcVertex]);

        if (idx < 0) {
            // new tc
            idx = allocateTcIndex(tcVertex, vertex);
            // map
            tcVertex = idx;
            return;
        }

        // already mapped tc, find matching vertex
        auto &mapping(tcMap[idx]);

        while (mapping.vertex != vertex) {
            // different vertex, check
            if (mapping.chain < 0) {
                // end of chain, make new entry
                const auto prev(idx);
                idx = allocateTcIndex(tcVertex, vertex);
                tcMap[prev].chain = idx;
                break;
            }

            // try next one
            idx = mapping.chain;
            mapping = tcMap[idx];
        }

        // map
        tcVertex = idx;
    });

    // copy faces, skip degenerate ones
    auto itc(facesTc.cbegin());
    for (const auto &face : faces) {
        // skip degenerate
        if ((face(0) == face(1)) || (face(1) == face(2))
            || (face(2) == face(0)))
        {
            if (hasTc) { ++itc; }
            continue;
        }

        // copy face, copy vertices, renumber indices
        ret.faces.push_back(face);
        auto &rface(ret.faces.back());
        for (int i = 0; i < 3; i++) { addVertex(rface(i)); }

        if (hasTc) {
            // same for texturing face but resolve different vertices mapping to
            // the same tc
            ret.facesTc.push_back(*itc++);
            auto &tface(ret.facesTc.back());
            for (int i = 0; i < 3; i++) {
                addTc(tface(i), rface(i));
            }
        }
    }

    return ret;
}

SubMesh& Mesh::add(const SubMesh &subMesh)
{
    bool simpleAdd(false);
    if (subMesh.tc.empty()) {
        // new submesh is texture-less
        simpleAdd = true;
    } else if (submeshes.empty()) {
        // empty mesh
        simpleAdd = true;
    } else if (!submeshes.back().tc.empty()) {
        // last submesh is textured
        simpleAdd = true;
    }

    if (simpleAdd) {
        submeshes.push_back(subMesh);
        return submeshes.back();
    }

    // ok, we have to insert new submesh before first non-textured existing
    // submesh

    auto isubmeshes(std::find_if(submeshes.begin(), submeshes.end()
                                 , [](const SubMesh &sm)
    {
        return sm.tc.empty();
    }));

    isubmeshes = submeshes.insert(isubmeshes, subMesh);
    return *isubmeshes;
}

void generateEtc(SubMesh &sm, const math::Extents2 &sdsExtents, bool allowed)
{
    sm.etc.clear();
    if (!allowed) { return; }

    TextureNormalizer tn(sdsExtents);

    // generate from vertices
    for (const auto &v : sm.vertices) {
        sm.etc.push_back(tn(v));
    }
}

void generateEtc(Mesh &mesh, const math::Extents2 &sdsExtents, bool allowed)
{
    if (!allowed) { return; }
    TextureNormalizer tn(sdsExtents);

    for (auto &sm : mesh) {
        sm.etc.clear();
        // generate from vertices
        for (const auto &v : sm.vertices) {
            sm.etc.push_back(tn(v));
        }
    }
}

} } // namespace vtslibs::vts
