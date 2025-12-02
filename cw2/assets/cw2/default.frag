#version 430

in vec3 v2fColor;
in vec3 v2fNormal;

layout ( location = 2 ) uniform vec3 uLightDir; // should be normalized! ∥uLightDir| = 1
layout ( location = 3 ) uniform vec3 uLightDiffuse;
layout ( location = 4 ) uniform vec3 uSceneAmbient;

layout( location = 0 ) out vec3 oColor;

void main()
{
    // v2fNormal is already normalized from vertex shader
    // vec3 normal = normalize(v2fNormal);

    float nDotL = max( 0.0, dot( v2fNormal, uLightDir ) );

    // Pre-compute lighting
    vec3 lighting = uSceneAmbient + nDotL * uLightDiffuse;

    oColor = lighting * v2fColor;
}
