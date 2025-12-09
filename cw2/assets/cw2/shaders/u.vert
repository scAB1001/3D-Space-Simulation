#version 430

// All possible vertex attributes (use 0 for unused)
layout(location = 0) in vec3 iPosition;    // Positions
layout(location = 1) in vec3 iColor;       // Colors (OR texcoords if materialType=1)
layout(location = 3 ) in vec3 iNormal;     // For lighting

// Transformation matrices
layout(location = 0) uniform mat4 uProjCameraWorld;
layout(location = 1) uniform mat3 uNormalMatrix;
layout(location = 10) uniform int uMaterialType; // 0 = colored, 1 = textured

// Lighting uniforms
layout(location = 2) uniform vec3 uLightDir;        // Directional light (normalised)
layout(location = 3) uniform vec3 uLightDiffuse;
layout(location = 4) uniform vec3 uSceneAmbient;

// Output to fragment shader
out vec3 v2fColor;
out vec2 v2fTexCoord;
out vec3 v2fDiffuse;
out vec3 v2fNormal;
flat out int v2fMaterialType;

void main()
{
    // Transform position
    gl_Position = uProjCameraWorld * vec4(iPosition, 1.0);

    // Transform normal for lighting
    v2fNormal = normalize(uNormalMatrix * iNormal);
    float nDotL = max(0.0, dot(v2fNormal, uLightDir));

    // Pre-calculate diffuse lighting in vertex shader
    v2fDiffuse = uSceneAmbient + nDotL * uLightDiffuse;

    // Pass attributes based on material type
    v2fMaterialType = uMaterialType;

    if (uMaterialType == 0)
    {
        // Colored object
        v2fColor = iColor;
        v2fTexCoord = vec2(0.0);
    }
    else if (uMaterialType == 1)
    {
        // Textured object - iColor contains texcoords
        v2fTexCoord = vec2(iColor.x, iColor.y); // Use only first 2 components
        v2fColor = vec3(1.0); // White base for textures
    }
    else
    {
        // Fallback
        v2fColor = vec3(1.0, 0.0, 1.0); // Magenta for debugging
        v2fTexCoord = vec2(0.0);
    }
}