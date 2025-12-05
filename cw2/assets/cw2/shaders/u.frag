#version 430

// Inputs from vertex shader
in vec3 v2fColor;
in vec2 v2fTexCoord;
in vec3 v2fDiffuse;
flat in int v2fMaterialType;

// Texture sampler
layout(binding = 0) uniform sampler2D uTexture;

// Output
layout(location = 0) out vec3 oColor;

void main()
{
    vec3 finalColor;

    // Determine base color based on material type
    if (v2fMaterialType == 0)
    {   // Colored object - use vertex color

        finalColor = v2fColor;
    }
    else if (v2fMaterialType == 1)
    {   // Textured object - sample texture

        finalColor = texture(uTexture, v2fTexCoord).rgb;

        // Debug: visualize texture coordinates
        // finalColor = vec3(v2fTexCoord, 0.0);
    }
    else
    {   // Debug: magenta for invalid material type

        finalColor = vec3(1.0, 0.0, 1.0);
    }

    // Apply lighting to final color
    oColor = v2fDiffuse * finalColor;

    // Debug: visualize normals
    // oColor = normal * 0.5 + 0.5;
}