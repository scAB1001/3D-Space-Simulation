#pragma once

#include "../vmlib/vec2.hpp"
#include "../vmlib/vec3.hpp"
#include <string>

struct UITextRenderer;

struct UIButton
{
    Vec2f pos;
    Vec2f size;
    std::string label;

    bool hovered = false;
    bool pressed = false;
    bool clicked = false;
};

// logic
void updateButton(
    UIButton& btn,
    float mouseX,
    float mouseY,
    bool mouseDown
);

// rendering
void drawButton(
    UIButton& btn,
    UITextRenderer& ui,
    float screenW,
    float screenH
);
