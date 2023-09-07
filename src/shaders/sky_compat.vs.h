vs=R"GLSL(
uniform mat4 uProjMat;
uniform mat4 uModelViewMat;

attribute vec3 aPos;
attribute vec2 aTex;

varying vec2 vTex;

void main()
{
    vTex = aTex;
    gl_Position = uProjMat * vec4(vec3(uModelViewMat * vec4(aPos, 0.0)), 1);
}
)GLSL";
