#version 430 core

in vec2 vUV;
out vec4 FragColor;

layout (binding = 0) uniform sampler2D uFont;

void main()
{
    float lum = texture(uFont, vUV).r;

    // optional cutoff for crisp text
    if (lum < 0.1)
        discard;

    FragColor = vec4(1.0, 1.0, 1.0, lum);
}
