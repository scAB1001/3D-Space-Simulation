#version 430

// ------------ INPUT ATTRIBUTES ------------
layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec3 iColor;      // rgb OR texcoord.xy
layout(location = 3) in vec3 iNormal;

// ------------ UNIFORMS ------------
layout(location = 0) uniform mat4 uProjCameraWorld;
layout(location = 1) uniform mat3 uNormalMatrix;
layout(location = 10) uniform int uMaterialType;

// We add the model matrix explicitly
uniform mat4 uModel;

// Directional-light uniforms
layout(location = 2) uniform vec3 uLightDir;
layout(location = 3) uniform vec3 uLightDiffuse;
layout(location = 4) uniform vec3 uSceneAmbient;

// ------------ OUTPUT TO FRAGMENT SHADER ------------
out vec3 v2fColor;
out vec2 v2fTexCoord;
out vec3 v2fDiffuse;       // directional light term
out vec3 v2fNormal;        // world normal
out vec3 v2fWorldPos;      // world-space position
flat out int v2fMaterialType;

void main()
{
    // Compute world position for lighting
    vec4 wp = uModel * vec4(iPosition, 1.0);
    v2fWorldPos = wp.xyz;

    // Final clip position
    gl_Position = uProjCameraWorld * vec4(iPosition, 1.0);

    // Normal → world-space
    v2fNormal = normalize(uNormalMatrix * iNormal);

    // Directional light diffuse term
    float nDotL = max(0.0, dot(v2fNormal, normalize(uLightDir)));
    v2fDiffuse = uSceneAmbient + nDotL * uLightDiffuse;

    // Material switch
    v2fMaterialType = uMaterialType;

    if (uMaterialType == 0)
    {
        v2fColor = iColor;  
        v2fTexCoord = vec2(0.0);
    }
    else
    {
        v2fTexCoord = iColor.xy;
        v2fColor = vec3(1.0);
    }
}
