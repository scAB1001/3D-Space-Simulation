#version 430

// ------------ INPUT FROM VERTEX SHADER ------------
in vec3 v2fColor;
in vec2 v2fTexCoord;
in vec3 v2fNormal;
in vec3 v2fWorldPos;
flat in int v2fMaterialType;

// ------------ UNIFORMS ------------
layout(binding = 0) uniform sampler2D uTexture;

uniform vec3 uCameraPos;

// Point lights (3)
uniform vec3 uPointPos[3];
uniform vec3 uPointColor[3];
uniform int  uPointEnabled[3];

// Directional light (Section 1.2: ambient + diffuse only)
uniform int  uDirEnabled;
uniform vec3 uLightDir;      // should be (0, 1, -1) normalized in C++
uniform vec3 uLightDiffuse;
uniform vec3 uSceneAmbient;

// Material
uniform vec3  uMaterialKd;        // diffuse Kd (for landing pad etc.)
uniform float uMaterialShininess; // Ns (used ONLY for point lights)

// ------------ OUTPUT ------------
layout(location = 0) out vec3 oColor;

void main()
{
    //------------------------------------------------------------------
    // MATERIAL BASE COLOR
    //------------------------------------------------------------------
    vec3 baseColor;

    if (v2fMaterialType == 0) {                   // coloured
        baseColor = v2fColor;
    }
    else if (v2fMaterialType == 1) {              // textured
        baseColor = texture(uTexture, v2fTexCoord).rgb;
    }
    else {
        baseColor = vec3(1.0, 0.0, 1.0);          // debug
    }

    // Apply MTL Kd (for landing pad); for other objects this is (1,1,1)
    baseColor *= uMaterialKd;

    //------------------------------------------------------------------
    // SURFACE PROPERTIES
    //------------------------------------------------------------------
    vec3 N = normalize(v2fNormal);
    vec3 V = normalize(uCameraPos - v2fWorldPos);

    vec3 result = vec3(0.0);

    //------------------------------------------------------------------
    // GLOBAL DIRECTIONAL LIGHT (Section 1.2: ambient + diffuse)
    //------------------------------------------------------------------
    if (uDirEnabled == 1)
    {
        // Use same convention as original coursework: L = uLightDir
        vec3 L = normalize(uLightDir);

        float diff = max(dot(N, L), 0.0);

        vec3 ambient = uSceneAmbient * baseColor;
        vec3 diffuse = diff * uLightDiffuse * baseColor;

        // No specular term for the global light (per spec).
        result += ambient + diffuse;
    }

    //------------------------------------------------------------------
    // LOCAL POINT LIGHTS (Task 1.6: full Blinn–Phong, 1/r^2)
    //------------------------------------------------------------------
    for (int i = 0; i < 3; i++)
    {
        if (uPointEnabled[i] == 0)
            continue;

        vec3 Lvec = uPointPos[i] - v2fWorldPos;
        float dist = length(Lvec);
        vec3 L = normalize(Lvec);

        // Standard 1 / r^2 attenuation
        float atten = 1.0 / (dist * dist);

        float diff = max(dot(N, L), 0.0);

        vec3 H = normalize(L + V);
        float spec = pow(max(dot(N, H), 0.0), uMaterialShininess);

        vec3 ambient  = 0.05 * baseColor;          // small local ambient
        vec3 diffuse  = diff * baseColor * uPointColor[i];
        vec3 specular = spec * uPointColor[i];

        result += (ambient + diffuse + specular) * atten;
    }

    //------------------------------------------------------------------
    // FINAL COLOR
    //------------------------------------------------------------------
    oColor = result;
}
