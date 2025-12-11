#version 430

in vec3 v2fColor;
in vec2 v2fTexCoord;
in vec3 v2fNormal;
in vec3 v2fWorldPos;
flat in int v2fMaterialType;

out vec4 outColor;

// ----- Lighting Uniforms -----
uniform vec3 uLightDir;
uniform vec3 uLightDiffuse;
uniform vec3 uSceneAmbient;

uniform vec3 uPointPos[3];
uniform vec3 uPointColor[3];
uniform int  uPointEnabled[3];

uniform int uGlobalDirLightEnabled;
uniform vec3 uCameraPos;

// Materials
uniform sampler2D uTexture;
uniform vec3 uMaterialKd = vec3(1.0);
uniform float uMaterialShininess = 32.0;

void main()
{
    vec3 N = normalize(v2fNormal);
    vec3 V = normalize(uCameraPos - v2fWorldPos);

    // Base colour
    vec3 baseColor =
        (v2fMaterialType == 0)
        ? v2fColor
        : texture(uTexture, v2fTexCoord).rgb;

    vec3 result = baseColor * uSceneAmbient;

    // ----- Directional light -----
    if (uGlobalDirLightEnabled == 1) {
        vec3 L = normalize(-uLightDir);
        float NdotL = max(dot(N, L), 0.0);

        // diffuse
        result += baseColor * uLightDiffuse * NdotL;

        // specular
        if (NdotL > 0.0) {
            vec3 H = normalize(L + V);
            float spec = pow(max(dot(N, H), 0.0), uMaterialShininess);
            result += spec * uLightDiffuse;
        }
    }

    // ----- Point lights -----
    for (int i = 0; i < 3; i++) {
        if (uPointEnabled[i] == 0) continue;

        vec3 L = normalize(uPointPos[i] - v2fWorldPos);
        float NdotL = max(dot(N, L), 0.0);

        result += baseColor * uPointColor[i] * NdotL;
    }

    outColor = vec4(result, 1.0);
}
