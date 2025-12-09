#version 430

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec4 aColor;

uniform vec2 uScreenSize;

out vec4 vColor;

void main() {
    // Convert to normalized device coordinates
    vec2 ndc = (aPosition * 2.0) / uScreenSize - 1.0;
    ndc.y = -ndc.y; // Flip Y for screen coordinates

    gl_Position = vec4(ndc, 0.0, 1.0);
    vColor = aColor;
}