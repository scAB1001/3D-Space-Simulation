#include "ui_text.hpp"

#include "../support/program.hpp"
#include "texture.hpp"
#include "renderer.hpp"

// format of font atlas
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

static float pxToClipX(float x, float screenW)
{
    return (x / screenW) * 2.0f - 1.0f;
}

static float pxToClipY(float y, float screenH)
{
    return (y / screenH) * 2.0f - 1.0f;
}


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

void UITextRenderer::renderText(
    const std::string& text,
    float x,
    float y,
    float scale,
    const Vec3f& color,
    float screenW,
    float screenH
)
{
    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "uUseTexture"), 1);
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

    float cursorX = pxToClipX(x, screenW);
    float cursorY = pxToClipY(y, screenH);

    //text size
    float charW = 0.04f * scale;
    float charH = 0.08f * scale;


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

void UITextRenderer::drawRect(
    float x, float y,
    float w, float h,
    const Vec3f& color,
    float alpha,
    float screenW,
    float screenH
)
{
    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "uUseTexture"), 0);
    float x0 = pxToClipX(x, screenW);
    float y0 = pxToClipY(y, screenH);
    float x1 = pxToClipX(x + w, screenW);
    float y1 = pxToClipY(y + h, screenH);

    float verts[] = {
        x0, y0, 0.f, 0.f,
        x1, y0, 0.f, 0.f,
        x1, y1, 0.f, 0.f,

        x0, y0, 0.f, 0.f,
        x1, y1, 0.f, 0.f,
        x0, y1, 0.f, 0.f
    };

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);

    GLint loc = glGetUniformLocation(program, "uColor");
    glUniform4f(loc, color.x, color.y, color.z, alpha);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void UITextRenderer::drawRectOutline(
    float x, float y,
    float w, float h,
    const Vec3f& color,
    float screenW,
    float screenH
)
{
    const float thickness = 52.0f; // pixels

    // Top edge
    drawRect(
        x,
        y + h - thickness,
        w,
        thickness,
        color,
        1.0f,
        screenW,
        screenH
    );

    // Bottom edge
    drawRect(
        x,
        y,
        w,
        thickness,
        color,
        1.0f,
        screenW,
        screenH
    );

    // Left edge
    drawRect(
        x,
        y,
        thickness,
        h,
        color,
        1.0f,
        screenW,
        screenH
    );

    // Right edge
    drawRect(
        x + w - thickness,
        y,
        thickness,
        h,
        color,
        1.0f,
        screenW,
        screenH
    );
}
