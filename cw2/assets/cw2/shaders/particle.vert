#version 430

layout(location = 0) in vec2 aQuad;

uniform mat4 uProj;
uniform mat4 uView;

uniform vec3 uParticlePos;
uniform vec3 uCameraRight;
uniform vec3 uCameraUp;

out vec2 vUV;

void main()
{
    // world-space billboard offset
    vec3 worldPos = uParticlePos +
                    aQuad.x * uCameraRight +
                    aQuad.y * uCameraUp;

    gl_Position = uProj * uView * vec4(worldPos, 1.0);

    vUV = aQuad * 0.5 + 0.5;
}
