#ifndef FPCAM_H
#define FPCAM_H

#include <glm/vec3.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4

class FPCam
{
public:
    void keyb(bool l, bool r, bool f, bool b, float dlr, float dfb);
    void mouse(int dx, int dy, float d);
    glm::mat4 transf() const;

    glm::vec3 p_pos = glm::vec3(0.f, 0.f, 0.f);
    float p_ax = 0.f;
    float p_ay = 0.f;
};

#endif // FPCAM_H
