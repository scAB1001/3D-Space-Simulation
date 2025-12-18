#ifndef UI_TEXT_HPP
#define UI_TEXT_HPP

#include <string>
#include "renderer.hpp"
#include "../vmlib/vec3.hpp"
#include "../support/program.hpp"

struct UITextRenderer
{
    // 16x16 ASCII atlas
    static constexpr float glyphW = 1.f / 16.f;
    static constexpr float glyphH = 1.f / 16.f;

    // Font atlas character mapping
    static constexpr int ATLAS_COLS = 10;
    static constexpr int ATLAS_ROWS = 10;
    static constexpr float GLYPH_W = 1.f / ATLAS_COLS;
    static constexpr float GLYPH_H = 1.f / ATLAS_ROWS;

    // UI button dimensions
    static constexpr float btnW = 180.f;
    static constexpr float btnH = 50.f;
    static constexpr float spacing = 20.f;
    static constexpr float marginBtm = 25.f;

    void init();
    void cleanup();

    void renderText(
        const std::string& text,
        float x,
        float y,
        float scale,
        const Vec3f& color,
        float screenW,
        float screenH
    );

    void renderAltitude(
        int altitude,
        float screenW,
        float screenH);

    void drawRect(
        float x, float y,
        float w, float h,
        const Vec3f& color,
        float alpha,
        float screenW,
        float screenH
    );

    void drawRectOutline(
        float x, float y,
        float w, float h,
        const Vec3f& color,
        float screenW,
        float screenH
    );

    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint fontTexture = 0;
    GLuint program = 0;
    ShaderProgram* shader = nullptr;
};

#endif // UI_TEXT_HPP