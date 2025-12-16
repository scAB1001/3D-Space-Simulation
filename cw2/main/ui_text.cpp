#include "ui_text.hpp"

#include "../support/program.hpp"
#include "texture.hpp"
#include "renderer.hpp"

#include <vector>

// 16x16 ASCII atlas
static constexpr float glyphW = 1.f / 16.f;
static constexpr float glyphH = 1.f / 16.f;

void UITextRenderer::init()
{
    shader = new ShaderProgram({
        { GL_VERTEX_SHADER,   "assets/cw2/shaders/ui.vert" },
        { GL_FRAGMENT_SHADER, "assets/cw2/shaders/ui.frag" }
    });

    program = shader->programId();

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

    // vec4: x, y, u, v  → location 0
    glVertexAttribPointer(
        0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0
    );
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    fontTexture = load_texture_2d("assets/cw2/font_atlas.png");
    glBindTexture(GL_TEXTURE_2D, fontTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

}

void UITextRenderer::cleanup()
{
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
    if (fontTexture) glDeleteTextures(1, &fontTexture);

    delete shader;
    shader = nullptr;

    vao = vbo = fontTexture = program = 0;
}

 
static const std::string ATLAS_CHARS =
    " !\"#$%&'()"
    "*+,-./0123"
    "456789:;<="
    ">?@ABCDEFG"
    "HIJKLMNOPQ"
    "RSTUVWXYZ["
    "\\]^_`abcde"
    "fghijklmno"
    "pqrstuvwxy"
    "z{|}~     ";

void UITextRenderer::renderText(
    const std::string& text,
    float,
    float,
    float,
    const Vec3f&,
    float,
    float
)
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fontTexture);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    constexpr int ATLAS_COLS = 10;
    constexpr int ATLAS_ROWS = 10;
    constexpr float GLYPH_W = 1.f / ATLAS_COLS;
    constexpr float GLYPH_H = 1.f / ATLAS_ROWS;

    float cursorX = -0.95f;   // top-left in clip space
    float cursorY =  0.90f;

    float charW = 0.04f;
    float charH = 0.08f;
    

    for (char c : text)
    {
        // force uppercase (ASCII-safe)
        if (c >= 'a' && c <= 'z')
            c = c - 'a' + 'A';

        auto pos = ATLAS_CHARS.find(c);

        if (pos == std::string::npos)
        {
            cursorX += charW;
            continue;
        }

        int col = pos % ATLAS_COLS;
        int row = pos / ATLAS_COLS;

        // Flip Y because PNG origin is top-left
        row = ATLAS_ROWS - 1 - row;
        constexpr float EPS = 0.002f;

        float u0 = col * GLYPH_W + EPS;
        float u1 = (col + 1) * GLYPH_W - EPS;
        float v0 = row * GLYPH_H + EPS;
        float v1 = (row + 1) * GLYPH_H - EPS;


        float verts[] = {
            cursorX,           cursorY,            u0, v1,
            cursorX + charW,   cursorY,            u1, v1,
            cursorX + charW,   cursorY - charH,    u1, v0,

            cursorX,           cursorY,            u0, v1,
            cursorX + charW,   cursorY - charH,    u1, v0,
            cursorX,           cursorY - charH,    u0, v0
        };

        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        cursorX += charW * 0.6f;
    }

    glBindVertexArray(0);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    RendererInternal::currentShaderProgram = 0;
}
