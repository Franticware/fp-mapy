vs=R"GLSL(
uniform mat4 uProjMat;
uniform mat4 uModelViewMat;

attribute vec3 aPos;

varying vec3 vTex;

void main()
{
    vTex = vec3(aPos.z, -aPos.y, aPos.x);
    gl_Position = uProjMat * vec4(vec3(uModelViewMat * vec4(aPos, 0.0)), 1);
}
)GLSL";
