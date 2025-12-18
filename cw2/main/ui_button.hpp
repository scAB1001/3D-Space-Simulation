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
void setButtonState(
    UIButton &btn,
    Vec2f position,
    Vec2f size,
    const std::string &label
);

void getNextButtonPos(
    const UIButton &lastBtn,
    Vec2f &outPos,
    float spacing
);

void updateButton(
    UIButton& btn,
    Vec2f position,
    Vec2f size,
    const std::string &label,
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
