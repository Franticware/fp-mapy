#ifndef CULLING_H
#define CULLING_H

#include <glm/glm.hpp>

bool cullingFrustumBox(const glm::mat4& mat, const glm::vec3& mid, const glm::vec3& ext);

#endif // CULLING_H
