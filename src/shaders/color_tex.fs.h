fs=R"GLSL(
uniform sampler2D uTex0;

varying vec2 vTex;
varying vec4 vColor;

void main()
{
    gl_FragColor = vec4(1, 1, 1, texture2D( uTex0, vTex ).a) * vColor;
}
)GLSL";
