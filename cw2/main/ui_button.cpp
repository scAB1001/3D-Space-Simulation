#include "ui_button.hpp"
#include "ui_text.hpp"

void updateButton(
    UIButton& btn,
    float mouseX,
    float mouseY,
    bool mouseDown
)
{
    btn.hovered =
        mouseX >= btn.pos.x &&
        mouseX <= btn.pos.x + btn.size.x &&
        mouseY >= btn.pos.y &&
        mouseY <= btn.pos.y + btn.size.y;

    btn.clicked = false;

    if (btn.hovered && mouseDown && !btn.pressed)
        btn.pressed = true;

    if (btn.pressed && !mouseDown)
    {
        if (btn.hovered)
            btn.clicked = true;

        btn.pressed = false;
    }
}

void drawButton(
    UIButton& btn,
    UITextRenderer& ui,
    float screenW,
    float screenH
)
{
    //Button colours
    Vec3f fill{0.25f, 0.25f, 0.25f};
    float alpha = 0.15f;

    if (btn.hovered)
        fill = {0.35f, 0.35f, 0.35f};

    if (btn.pressed)
        fill = {0.15f, 0.15f, 0.15f};

    // button size
    ui.drawRect(
        btn.pos.x,
        btn.pos.y,
        btn.size.x,
        btn.size.y,
        fill,
        alpha,
        screenW,
        screenH
    );

    // outline
    ui.drawRectOutline(
        btn.pos.x,
        btn.pos.y,
        btn.size.x,
        btn.size.y,
        Vec3f{0.f, 0.f, 0.f},
        screenW,
        screenH
    );

    //  CENTERED TEXT
    float textScale = 1.0f;

    // Ensure UITextRenderer glyph sizing matches
    float glyphW = 12.f * textScale;
    float glyphH = 24.f * textScale;

    float textWidth  = btn.label.size() * glyphW;
    float textHeight = glyphH;

    float textX = btn.pos.x + (btn.size.x - textWidth) * 0.4f;
    float textY = btn.pos.y + (btn.size.y - textHeight) * 1.5f;

    ui.renderText(
        btn.label,
        textX,
        textY,
        textScale,
        Vec3f{0.f, 0.f, 0.f}, // black text
        screenW,
        screenH
    );
}
