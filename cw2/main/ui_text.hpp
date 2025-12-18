#ifndef UI_TEXT_HPP
#define UI_TEXT_HPP

#include <string>
#include "../vmlib/vec3.hpp"
#include "../support/program.hpp"

struct UITextRenderer
{
    // 16x16 ASCII atlas
    static constexpr float glyphW = 1.f / 16.f;
    static constexpr float glyphH = 1.f / 16.f;

    void init();

    void renderText(
        const std::string& text,
        float x,
        float y,
        float scale,
        const Vec3f& color,
        float screenW,
        float screenH
    );

    void cleanup();

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