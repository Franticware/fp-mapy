fs=R"GLSL(
uniform samplerCube uCube;

varying vec3 vTex;

void main()
{
    gl_FragColor = textureCube(uCube, vTex);
}
)GLSL";
