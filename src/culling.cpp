#include "culling.h"

#include <cstdio>

bool cullingFrustumBox(const glm::mat4& mat, const glm::vec3& mid, const glm::vec3& ext)
{
    const glm::vec3 bboxVertMul[8] = {
        glm::vec3( 1.f,  1.f,  1.f),
        glm::vec3(-1.f,  1.f,  1.f),
        glm::vec3( 1.f, -1.f,  1.f),
        glm::vec3(-1.f, -1.f,  1.f),
        glm::vec3( 1.f,  1.f, -1.f),
        glm::vec3(-1.f,  1.f, -1.f),
        glm::vec3( 1.f, -1.f, -1.f),
        glm::vec3(-1.f, -1.f, -1.f),
    };
    bool cullFlag[6] = {true, true, true, true, true, true}; // culling flag from clip plane standpoint
    for (size_t i = 0; i != 8; ++i)
    {
        glm::vec4 vert = mat * glm::vec4(mid + bboxVertMul[i] * ext, 1.f); // vert.w can be negative! Do not divide by it!
        if (vert.x > -vert.w) cullFlag[0] = false;
        if (vert.x < vert.w) cullFlag[1] = false;
        if (vert.y > -vert.w) cullFlag[2] = false;
        if (vert.y < vert.w) cullFlag[3] = false;
        if (vert.z > -vert.w) cullFlag[4] = false;
        if (vert.z < vert.w) cullFlag[5] = false;
    }
    return !(cullFlag[0] || cullFlag[1] || cullFlag[2] || cullFlag[3] || cullFlag[4] || cullFlag[5]); // Step 1: Is at least one plane culling the box? Step 2: Flip meaning (cull? -> visible?).
}
