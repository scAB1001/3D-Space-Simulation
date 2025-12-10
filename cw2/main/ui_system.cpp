#include "ui_system.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <print>

// Simple shader for UI rectangles
const char *uiVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
uniform mat4 projection;
void main()
{
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
}
)";

const char *uiFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec4 color;
void main()
{
    FragColor = color;
}
)";

UISystem::UISystem() : screenWidth(1280), screenHeight(720), uiShaderProgram(0), uiVAO(0), uiVBO(0) {}

UISystem::~UISystem()
{
    if (uiVAO)
        glDeleteVertexArrays(1, &uiVAO);
    if (uiVBO)
        glDeleteBuffers(1, &uiVBO);
    if (uiShaderProgram)
        glDeleteProgram(uiShaderProgram);
}

bool UISystem::init(const char *fontPath)
{
    // Compile UI shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &uiVertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &uiFragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    uiShaderProgram = glCreateProgram();
    glAttachShader(uiShaderProgram, vertexShader);
    glAttachShader(uiShaderProgram, fragmentShader);
    glLinkProgram(uiShaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Create UI VAO/VBO
    glGenVertexArrays(1, &uiVAO);
    glGenBuffers(1, &uiVBO);

    return fontRenderer.init(fontPath, 18);
}

void UISystem::setScreenSize(int width, int height)
{
    screenWidth = width;
    screenHeight = height;
    fontRenderer.setScreenSize(width, height);
}

void UISystem::update(float mouseX, float mouseY, bool mousePressed)
{
    for (auto &[id, button] : buttons)
    {
        if (button.isVisible)
            button.update(mouseX, mouseY, mousePressed);
    }
}

void UISystem::draw()
{
    // Save OpenGL state
    GLboolean prevBlendEnabled = glIsEnabled(GL_BLEND);
    GLint prevBlendSrc, prevBlendDst;
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &prevBlendSrc);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &prevBlendDst);
    GLboolean prevDepthTest = glIsEnabled(GL_DEPTH_TEST);
    GLboolean prevCullFace = glIsEnabled(GL_CULL_FACE);

    // Enable blending, disable depth test
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // Draw all buttons
    for (const auto &[id, button] : buttons)
    {
        if (button.isVisible)
            button.draw(fontRenderer, uiShaderProgram, uiVAO, uiVBO, screenWidth, screenHeight);
    }

    // Restore OpenGL state
    if (!prevBlendEnabled)
        glDisable(GL_BLEND);
    glBlendFunc(prevBlendSrc, prevBlendDst);
    if (prevDepthTest)
        glEnable(GL_DEPTH_TEST);
    if (prevCullFace)
        glEnable(GL_CULL_FACE);
}

void UISystem::addButton(const std::string &id, UIButton button)
{
    buttons[id] = button;
}

UIButton *UISystem::getButton(const std::string &id)
{
    auto it = buttons.find(id);
    return it != buttons.end() ? &it->second : nullptr;
}

void UISystem::drawText(const std::string &text, float x, float y, const Vec4f &color)
{
    // Save state
    GLboolean prevDepthTest = glIsEnabled(GL_DEPTH_TEST);
    GLboolean prevBlendEnabled = glIsEnabled(GL_BLEND);
    GLint prevBlendSrc, prevBlendDst;
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &prevBlendSrc);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &prevBlendDst);

    // Setup for text rendering
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw text
    fontRenderer.drawText(text, x, y, color);

    // Restore state
    if (!prevBlendEnabled)
        glDisable(GL_BLEND);
    glBlendFunc(prevBlendSrc, prevBlendDst);
    if (prevDepthTest)
        glEnable(GL_DEPTH_TEST);
}

void UISystem::drawRect(float x, float y, float w, float h, const Vec4f &color) const
{
    if (uiShaderProgram == 0)
        return;

    float vertices[] = {
        x, y,
        x + w, y,
        x + w, y + h,
        x, y,
        x + w, y + h,
        x, y + h};

    // Set up projection matrix
    float projection[16] = {
        2.0f / screenWidth, 0.0f, 0.0f, -1.0f,
        0.0f, -2.0f / screenHeight, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f};

    glUseProgram(uiShaderProgram);

    // Set projection matrix
    GLuint projLoc = glGetUniformLocation(uiShaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection);

    // Set color
    GLuint colorLoc = glGetUniformLocation(uiShaderProgram, "color");
    glUniform4f(colorLoc, color.x, color.y, color.z, color.w);

    // Draw rectangle
    glBindVertexArray(uiVAO);
    glBindBuffer(GL_ARRAY_BUFFER, uiVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    glUseProgram(0);
}

bool UIButton::contains(float x, float y) const
{
    return x >= position.x - size.x / 2 && x <= position.x + size.x / 2 &&
           y >= position.y - size.y / 2 && y <= position.y + size.y / 2;
}

void UIButton::draw(FontRenderer &font, GLuint shaderProgram, GLuint vao, GLuint vbo, int screenWidth, int screenHeight) const
{
    if (!isVisible)
        return;

    // Choose color based on state
    Vec4f fillColor = isPressed ? colorPressed : (isHovered ? colorHover : colorNormal);

    // Draw button background
    float x = position.x - size.x / 2;
    float y = position.y - size.y / 2;

    // Draw fill using modern OpenGL
    float vertices[] = {
        x, y,
        x + size.x, y,
        x + size.x, y + size.y,
        x, y,
        x + size.x, y + size.y,
        x, y + size.y};

    // Set up projection matrix
    float projection[16] = {
        2.0f / screenWidth, 0.0f, 0.0f, -1.0f,
        0.0f, -2.0f / screenHeight, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f};

    glUseProgram(shaderProgram);

    // Set projection matrix
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection);

    // Set color
    GLuint colorLoc = glGetUniformLocation(shaderProgram, "color");
    glUniform4f(colorLoc, fillColor.x, fillColor.y, fillColor.z, fillColor.w);

    // Draw rectangle
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Draw outline (using lines)
    float outlineVertices[] = {
        x, y,
        x + size.x, y,
        x + size.x, y,
        x + size.x, y + size.y,
        x + size.x, y + size.y,
        x, y + size.y,
        x, y + size.y,
        x, y};

    glUniform4f(colorLoc, outlineColor.x, outlineColor.y, outlineColor.z, outlineColor.w);
    glBufferData(GL_ARRAY_BUFFER, sizeof(outlineVertices), outlineVertices, GL_STATIC_DRAW);
    glDrawArrays(GL_LINES, 0, 8);

    glBindVertexArray(0);
    glUseProgram(0);

    // Draw centered text
    Vec2f textSize = font.getTextSize(label);
    float textX = position.x - textSize.x / 2;
    float textY = position.y - textSize.y / 2;

    font.drawText(label, textX, textY, textColor);
}

void UIButton::update(float mouseX, float mouseY, bool mousePressed)
{
    bool wasPressed = isPressed;
    isHovered = contains(mouseX, mouseY);

    if (isHovered && mousePressed)
        isPressed = true;
    else
        isPressed = false;

    // Trigger click on release
    if (wasPressed && !isPressed && isHovered && onClick)
        onClick();
}