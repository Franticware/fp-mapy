vs=R"GLSL(
uniform mat4 uProjModelViewMat;

attribute vec3 aPos;

void main()
{
    gl_Position = uProjModelViewMat * vec4(aPos, 1.0);
}
)GLSL";
