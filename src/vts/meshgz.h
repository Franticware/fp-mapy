#ifndef MESHGZ_H
#define MESHGZ_H

#include <istream>
#include <cstdio>
#include <vector>
#include <cstdint>
#include "vts/mesh.hpp"

int loadMeshZ(const char* fname, vtslibs::vts::Mesh& mesh);
int loadMeshZ(const std::vector<uint8_t>& inData, vtslibs::vts::Mesh& mesh);

#endif // MESHGZ_H
