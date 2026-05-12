#version 430

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uTex;
uniform vec4 uColor;
uniform int uUseTexture; // 0 = solid, 1 = text

void main()
{
    /* References for code inspiration:
     * - https://learnopengl.com/Getting-started/Shaders
     * - https://wikis.khronos.org/opengl/Uniform_(GLSL)
     * - https://learnopengl.com/Advanced-Lighting/Advanced-Lighting
     */

    if (uUseTexture == 1)
    {
        vec4 tex = texture(uTex, vUV);
        FragColor = vec4(uColor.rgb, uColor.a * tex.r);
    }
    else
    {
        FragColor = uColor;
    }
}
