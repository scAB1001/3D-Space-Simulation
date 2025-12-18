#version 430

layout (location = 0) in vec4 aPosUV;

out vec2 vUV;

void main()
{
    /* References for code inspiration:
     * - https://learnopengl.com/Getting-started/Shaders
     * - https://wikis.khronos.org/opengl/Uniform_(GLSL)
     * - https://learnopengl.com/Advanced-Lighting/Advanced-Lighting
     */
     
    gl_Position = vec4(aPosUV.xy, 0.0, 1.0);
    vUV = aPosUV.zw;
}
