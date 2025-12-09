#version 430

in vec2 vTexCoord;
in vec4 vColor;

uniform sampler2D uTexture;

out vec4 oColor;

void main() {
    float alpha = texture(uTexture, vTexCoord).r;
    oColor = vec4(vColor.rgb, vColor.a * alpha);
}