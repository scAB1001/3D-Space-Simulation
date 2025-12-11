#version 430

// ------------ INPUT ATTRIBUTES ------------
layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec3 iColor;      // rgb OR texcoord.xy
layout(location = 3) in vec3 iNormal;

// ------------ UNIFORMS ------------
layout(location = 0) uniform mat4 uProjCameraWorld;
layout(location = 1) uniform mat3 uNormalMatrix;
layout(location = 10) uniform int uMaterialType;

uniform mat4 uModel;

// Directional light uniforms
layout(location = 2) uniform vec3 uLightDir;
layout(location = 3) uniform vec3 uLightDiffuse;
layout(location = 4) uniform vec3 uSceneAmbient;

// ------------ OUTPUT TO FRAGMENT SHADER ------------
out vec3 v2fColor;
out vec2 v2fTexCoord;
out vec3 v2fNormal;
out vec3 v2fWorldPos;
flat out int v2fMaterialType;

void main()
{
    // World position for lighting
    vec4 wp = uModel * vec4(iPosition, 1.0);
    v2fWorldPos = wp.xyz;

    // Clip-space position
    gl_Position = uProjCameraWorld * vec4(iPosition, 1.0);

    // Normal in world space
    v2fNormal = normalize(uNormalMatrix * iNormal);

    // Material type
    v2fMaterialType = uMaterialType;

    if (uMaterialType == 0) {
        // coloured
        v2fColor = iColor;
        v2fTexCoord = vec2(0.0);
    }
    else {
        // textured: iColor.xy stores texcoord
        v2fTexCoord = iColor.xy;
        v2fColor = vec3(1.0);
    }
}
