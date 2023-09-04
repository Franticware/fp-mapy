#include "fpcam.h"

#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale

#include <cmath>

void FPCam::keyb(bool l, bool r, bool f, bool b, float dlr, float dfb)
{
    int lr = 0;
    int fb = 0;
    if (l) --lr;
    if (r) ++lr;
    if (f) ++fb;
    if (b) --fb;

    const float sin_ax = std::sin(p_ax);
    const float cos_ax = std::cos(p_ax);
    const float sin_ay = std::sin(p_ay);
    const float cos_ay = std::cos(p_ay);

    p_pos[0] += cos_ay*lr*dlr;
    p_pos[2] -= sin_ay*lr*dlr;

    p_pos[0] -= sin_ay*cos_ax*fb*dfb;
    p_pos[1] += sin_ax*fb*dfb;
    p_pos[2] -= cos_ay*cos_ax*fb*dfb;
}

void FPCam::mouse(int dx, int dy, float d)
{
    p_ax -= dy * d;

    if (p_ax < -M_PI_2)
    {
        p_ax = -M_PI_2;
    }
    else if (p_ax > M_PI_2)
    {
        p_ax = M_PI_2;
    }

    p_ay -= dx * d;

    if (p_ay < -M_PI)
    {
        p_ay += M_PI * 2;
    }
    else if (p_ay > M_PI)
    {
        p_ay -= M_PI * 2;
    }
}

glm::mat4 FPCam::transf() const
{
    glm::mat4 ret(1.f);
    ret = glm::rotate(ret, -p_ax, glm::vec3(1.f, 0.f, 0.f));
    ret = glm::rotate(ret, -p_ay, glm::vec3(0.f, 1.f, 0.f));
    ret = glm::translate(ret, -p_pos);
    return ret;
}
