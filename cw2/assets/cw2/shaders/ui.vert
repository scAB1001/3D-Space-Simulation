#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec4 aColor;

out vec2 TexCoord;
out vec4 Color;

uniform mat4 projection;
uniform int drawMode;  // 0 = colored rect, 1 = textured (text)

void main()
{
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
    Color = aColor;
}