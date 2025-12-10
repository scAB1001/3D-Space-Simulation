#ifndef UI_SYSTEM_HPP
#define UI_SYSTEM_HPP

#include <string>
#include <functional>
#include <unordered_map>
#include "../vmlib/vec2.hpp"
#include "../vmlib/vec4.hpp"
#include "font_renderer.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Simple UI Button
struct UIButton
{
    std::string label;
    Vec2f position;             // Center position in pixels
    Vec2f size = {100.f, 40.f}; // Width, Height

    // Visual states
    Vec4f colorNormal = {0.2f, 0.2f, 0.2f, 0.8f};
    Vec4f colorHover = {0.3f, 0.3f, 0.3f, 0.9f};
    Vec4f colorPressed = {0.4f, 0.4f, 0.4f, 1.0f};
    Vec4f outlineColor = {1.0f, 1.0f, 1.0f, 1.0f};
    Vec4f textColor = {1.0f, 1.0f, 1.0f, 1.0f};

    // State
    bool isHovered = false;
    bool isPressed = false;
    bool isVisible = true;

    // Callback
    std::function<void()> onClick;

    // Check if point is inside button
    bool contains(float x, float y) const;

    // Draw the button (updated parameters)
    void draw(FontRenderer &font, GLuint shaderProgram, GLuint vao, GLuint vbo, int screenWidth, int screenHeight) const;

    // Update state based on mouse
    void update(float mouseX, float mouseY, bool mousePressed);
};

// Simple UI System
class UISystem
{
public:
    UISystem();
    ~UISystem();

    // Initialize with font
    bool init(const char *fontPath);

    // Update screen size (call when window resizes)
    void setScreenSize(int width, int height);

    // Update UI (call before drawing)
    void update(float mouseX, float mouseY, bool mousePressed);

    // Draw all UI elements
    void draw();

    // Add UI elements
    void addButton(const std::string &id, UIButton button);
    UIButton *getButton(const std::string &id);

    // Draw text at position (in pixels)
    void drawText(const std::string &text, float x, float y, const Vec4f &color = {1.f, 1.f, 1.f, 1.f});

private:
    FontRenderer fontRenderer;
    std::unordered_map<std::string, UIButton> buttons;
    int screenWidth, screenHeight;

    // OpenGL resources for UI drawing
    GLuint uiShaderProgram;
    GLuint uiVAO;
    GLuint uiVBO;

    // Simple 2D rendering
    void drawRect(float x, float y, float w, float h, const Vec4f &color) const;
};

#endif // UI_SYSTEM_HPP