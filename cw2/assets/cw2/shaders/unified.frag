#version 430

// ------------ INPUT FROM VERTEX SHADER ------------
in vec3 v2fColor;
in vec2 v2fTexCoord;
in vec3 v2fDiffuse;
in vec3 v2fNormal;
in vec3 v2fWorldPos;
flat in int v2fMaterialType;

// ------------ UNIFORMS ------------
layout(binding = 0) uniform sampler2D uTexture;

uniform vec3 uCameraPos;

uniform vec3 uPointPos[3];
uniform vec3 uPointColor[3];
uniform int  uPointEnabled[3];

uniform int uDirEnabled; // 1 = use directional, 0 = disable

// ------------ OUTPUT ------------
layout(location = 0) out vec3 oColor;

void main()
{
    vec3 baseColor;

    // Material selection
    if (v2fMaterialType == 0)
        baseColor = v2fColor;
    else if (v2fMaterialType == 1)
        baseColor = texture(uTexture, v2fTexCoord).rgb;
    else
        baseColor = vec3(1.0, 0.0, 1.0);  // debug magenta

    // ------------------------
    // Directional lighting
    // ------------------------
    vec3 lighting = vec3(0.0);
    if (uDirEnabled == 1)
        lighting += v2fDiffuse;

    // ------------------------
    // Point lights
    // ------------------------
    vec3 N = normalize(v2fNormal);

    for (int i = 0; i < 3; i++)
    {
        if (uPointEnabled[i] == 0)
            continue;

        vec3 L = normalize(uPointPos[i] - v2fWorldPos);
        float diff = max(dot(N, L), 0.0);

        float dist = length(uPointPos[i] - v2fWorldPos);
        float atten = 1.0 / (dist * dist);

        lighting += diff * atten * uPointColor[i];
    }

    // final shading
    oColor = lighting * baseColor;
}
