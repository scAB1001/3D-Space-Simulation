#version 430 core

layout (location = 0) in vec4 aPosUV;

out vec2 vUV;

void main()
{
    gl_Position = vec4(aPosUV.xy, 0.0, 1.0);
    vUV = aPosUV.zw;
}
