// #pragma once

// #include <string>
// #include "../vmlib/vec2.hpp"

// enum class UIButtonState
// {
//     Normal,
//     Hover,
//     Pressed
// };

// struct UIButton
// {
//     Vec2f pos;          // bottom-left in pixels
//     Vec2f size;         // width / height in pixels
//     std::string label;

//     UIButtonState state = UIButtonState::Normal;

//     bool hovered = false;
//     bool pressed = false;
//     bool clicked = false;
// };

// // Updates hover / pressed / clicked
// void updateButton(
//     UIButton& btn,
//     float mouseX,
//     float mouseY,
//     bool mouseDown
// );

// // Draws button (implemented elsewhere)
// struct UITextRenderer;
// void drawButton(
//     UIButton& btn,
//     UITextRenderer& text,
//     float screenW,
//     float screenH
// );
