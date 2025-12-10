#version 330 core

in vec2 TexCoord;
in vec4 Color;

out vec4 FragColor;

uniform sampler2D uiTexture;
uniform int drawMode;  // 0 = colored rect, 1 = textured (text)

void main()
{
    if (drawMode == 0) {
        // Solid color rectangle
        FragColor = Color;
    } else {
        // Text rendering
        float alpha = texture(uiTexture, TexCoord).r;
        FragColor = Color * vec4(1.0, 1.0, 1.0, alpha);
    }
}