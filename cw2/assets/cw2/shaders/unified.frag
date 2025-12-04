#version 430

// Inputs from vertex shader
in vec3 v2fColor;
in vec2 v2fTexCoord;
in vec3 v2fNormal; // Normalised
flat in int v2fMaterialType;

// Lighting uniforms
layout(location = 2) uniform vec3 uLightDir;        // Directional light (normalised)
layout(location = 3) uniform vec3 uLightDiffuse;
layout(location = 4) uniform vec3 uSceneAmbient;

// Texture sampler
layout(binding = 0) uniform sampler2D uTexture;

// Output
layout(location = 0) out vec3 oColor;

void main()
{
    vec3 finalColor;

    // Determine base color based on material type
    if (v2fMaterialType == 0) {
        // Colored object - use vertex color
        finalColor = v2fColor;
    } else if (v2fMaterialType == 1) {
        // Textured object - sample texture
        finalColor = texture(uTexture, v2fTexCoord).rgb;

        // Debug: visualize texture coordinates
        // finalColor = vec3(v2fTexCoord, 0.0);
    } else {
        // Debug: magenta for invalid material type
        finalColor = vec3(1.0, 0.0, 1.0);
    }

    // Calculate lighting (simple diffuse + ambient)
    float nDotL = max(0.0, dot(v2fNormal, uLightDir));

    // Combine ambient and diffuse lighting
    vec3 lighting = uSceneAmbient + nDotL * uLightDiffuse;

    // Apply lighting to final color
    oColor = lighting * finalColor;

    // Debug: visualize normals
    // oColor = normal * 0.5 + 0.5;
}