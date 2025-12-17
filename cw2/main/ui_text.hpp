#pragma once

#include <string>
#include "../vmlib/vec3.hpp"
#include "../support/program.hpp"
struct UITextRenderer
{
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
