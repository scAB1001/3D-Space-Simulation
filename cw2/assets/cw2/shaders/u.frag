#version 430

in vec3 v2fColor;
in vec2 v2fTexCoord;
in vec3 v2fNormal;
in vec3 v2fWorldPos;
flat in int v2fMaterialType;

layout(binding = 0) uniform sampler2D uTexture;

uniform vec3 uCameraPos;

// global directional light
uniform int  uDirEnabled;
uniform vec3 uLightDir;       // must be normalized in C++
uniform vec3 uLightDiffuse;
uniform vec3 uSceneAmbient;

// point lights
uniform vec3 uPointPos[3];
uniform vec3 uPointColor[3];
uniform int  uPointEnabled[3];

// material
uniform vec3  uMaterialKd;
uniform float uMaterialShininess;

layout(location = 0) out vec3 oColor;

void main()
{
    vec3 baseColor;

    if (v2fMaterialType == 0)
        baseColor = v2fColor;
    else if (v2fMaterialType == 1)
        baseColor = texture(uTexture, v2fTexCoord).rgb;
    else
        baseColor = vec3(1.0, 0.0, 1.0);

    baseColor *= uMaterialKd;

    vec3 N = normalize(v2fNormal);
    vec3 V = normalize(uCameraPos - v2fWorldPos);

    vec3 result = vec3(0.0);

    // ----------------------------------------------------------------
    // GLOBAL DIRECTIONAL LIGHT: ambient + diffuse ONLY (CW2 requirement)
    // ----------------------------------------------------------------
    if (uDirEnabled == 1)
    {
        vec3 L = normalize(uLightDir);   // IMPORTANT: use positive L, not -L

        float diff = max(dot(N, L), 0.0);

        vec3 ambient = uSceneAmbient * baseColor;
        vec3 diffuse = diff * uLightDiffuse * baseColor;

        result += ambient + diffuse;
    }

    // ----------------------------------------------------------------
    // POINT LIGHTS : full Blinn–Phong
    // ----------------------------------------------------------------
    for (int i = 0; i < 3; i++)
    {
        if (uPointEnabled[i] == 0)
            continue;

        vec3 Lvec = uPointPos[i] - v2fWorldPos;
        float dist = length(Lvec);
        vec3 L = Lvec / dist;

        float atten = 1.0 / (dist * dist);

        float diff = max(dot(N, L), 0.0);

        vec3 H = normalize(L + V);
        float spec = pow(max(dot(N, H), 0.0), uMaterialShininess);

        vec3 ambient  = 0.05 * baseColor;
        vec3 diffuse  = diff * baseColor * uPointColor[i];
        vec3 specular = spec * uPointColor[i];

        result += (ambient + diffuse + specular) * atten;
    }

    oColor = result;
}
