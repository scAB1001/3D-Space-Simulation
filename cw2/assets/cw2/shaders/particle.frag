#version 430

in vec2 vUV;
out vec4 outColor;

uniform sampler2D uTexture;
uniform float uAlpha;

void main()
{
    /* References for code inspiration:
     * - https://wikis.khronos.org/opengl/Primitive#Point_primitives
     * - https://www.kenney.nl/assets/particle-pack
     */

    vec4 tex = texture(uTexture, vUV);

    if (tex.a < 0.1)
        discard;

    // Apply orange tint
    vec3 orange = vec3(1.0, 0.5, 0.0);

    // Apply orange tint
    vec3 finalColor = tex.rgb * orange;

    outColor = vec4(finalColor, tex.a * uAlpha);
}
