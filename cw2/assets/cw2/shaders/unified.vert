#version 430

layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec3 iColor;      // rgb or texcoord.xy
layout(location = 3) in vec3 iNormal;

layout(location = 0) uniform mat4 uProjCameraWorld;
layout(location = 1) uniform mat3 uNormalMatrix;
layout(location = 10) uniform int uMaterialType;

uniform mat4 uModel;

// Pass-through only
out vec3 v2fColor;
out vec2 v2fTexCoord;
out vec3 v2fNormal;
out vec3 v2fWorldPos;
flat out int v2fMaterialType;

void main()
{
    vec4 wp = uModel * vec4(iPosition, 1.0);
    v2fWorldPos = wp.xyz;

    gl_Position = uProjCameraWorld * vec4(iPosition, 1.0);

    v2fNormal = normalize(uNormalMatrix * iNormal);

    v2fMaterialType = uMaterialType;

    if (uMaterialType == 0) {
        v2fColor = iColor;
        v2fTexCoord = vec2(0.0);
    }
    else {
        v2fTexCoord = iColor.xy;
        v2fColor = vec3(1.0);
    }
}
