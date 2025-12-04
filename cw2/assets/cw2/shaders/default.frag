#version 430

// Inputs from vertex shader
in vec3 v2fColor;
in vec3 v2fNormal; // already normalised

// Uniforms
layout( location = 2 ) uniform vec3 uLightDir; // normalised |uLightDir| = 1
layout( location = 3 ) uniform vec3 uLightDiffuse;
layout( location = 4 ) uniform vec3 uSceneAmbient;

// Output
layout( location = 0 ) out vec3 oColor;

void main()
{
    // Simplified shading model based on (Blinn-)Phong shading
    // Uses a directional light
    float nDotL = max( 0.0, dot( v2fNormal, uLightDir ) );

    // Pre-compute ambient and diffuse lighting
    vec3 lighting = uSceneAmbient + nDotL * uLightDiffuse;

    // Apply lighting to vertex color
    oColor = lighting * v2fColor;

    // For debugging: visualize normals
    // oColor = v2fNormal * 0.5 + 0.5;
}
