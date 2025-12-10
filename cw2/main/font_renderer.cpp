#include "font_renderer.hpp"

// Define these BEFORE including fontstash headers
#define FONTSTASH_IMPLEMENTATION
#define FONS_IMPLEMENTATION

// Now include fontstash
#include "fontstash.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <print>

// Define the FontData struct (now it's accessible in this file)
struct FontRenderer::FontData
{
    FONScontext *fs = nullptr;
    GLuint texture = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint shaderProgram = 0;
    int font = -1;

    // Store screen dimensions for projection matrix
    int screenWidth = 1280;
    int screenHeight = 720;
};

// Simple shader sources
const char *vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec4 aColor;

out vec2 TexCoord;
out vec4 Color;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
    Color = aColor;
}
)";

const char *fragmentShaderSource = R"(
#version 330 core
in vec2 TexCoord;
in vec4 Color;
out vec4 FragColor;

uniform sampler2D textTexture;

void main()
{
    float alpha = texture(textTexture, TexCoord).r;
    FragColor = Color * vec4(1.0, 1.0, 1.0, alpha);
}
)";

GLuint compileShader(const char *source, GLenum type)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
    }
    return shader;
}

FontRenderer::FontRenderer() : data(new FontData), screenWidth(1280), screenHeight(720)
{
    data->screenWidth = screenWidth;
    data->screenHeight = screenHeight;
}

FontRenderer::~FontRenderer()
{
    if (data->fs)
        fonsDeleteInternal(data->fs);
    if (data->texture)
        glDeleteTextures(1, &data->texture);
    if (data->vao)
        glDeleteVertexArrays(1, &data->vao);
    if (data->vbo)
        glDeleteBuffers(1, &data->vbo);
    if (data->shaderProgram)
        glDeleteProgram(data->shaderProgram);
    delete data;
}

struct Vertex
{
    float x, y;
    float u, v;
    unsigned int color;
};

// Static callback implementations
void FontRenderer::renderDrawImpl(void *userPtr, const float *verts, const float *tcoords, const unsigned int *colors, int nverts)
{
    FontRenderer::FontData *data = static_cast<FontRenderer::FontData *>(userPtr);
    std::print("Drawing {} vertices, texture: {}\n", nverts, data->texture);
    if (data->vao == 0 || data->vbo == 0)
    {
        // Create VAO and VBO if not already created
        glGenVertexArrays(1, &data->vao);
        glGenBuffers(1, &data->vbo);
    }

    // Convert Fontstash vertices to our vertex format
    std::vector<Vertex> vertices(nverts);
    for (int i = 0; i < nverts; i++)
    {
        vertices[i].x = verts[i * 2];
        vertices[i].y = verts[i * 2 + 1];
        vertices[i].u = tcoords[i * 2];
        vertices[i].v = tcoords[i * 2 + 1];
        vertices[i].color = colors[i];
    }

    // Upload vertex data
    glBindVertexArray(data->vao);
    glBindBuffer(GL_ARRAY_BUFFER, data->vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);

    // Set vertex attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, x));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, u));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void *)offsetof(Vertex, color));

    // Compile shader if needed
    if (data->shaderProgram == 0)
    {
        GLuint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
        GLuint fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

        data->shaderProgram = glCreateProgram();
        glAttachShader(data->shaderProgram, vertexShader);
        glAttachShader(data->shaderProgram, fragmentShader);
        glLinkProgram(data->shaderProgram);

        GLint success;
        glGetProgramiv(data->shaderProgram, GL_LINK_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetProgramInfoLog(data->shaderProgram, 512, NULL, infoLog);
            std::cerr << "Shader linking failed: " << infoLog << std::endl;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, data->texture);

    // Use shader
    glUseProgram(data->shaderProgram);

    // Set projection matrix (orthographic)
    float projection[16] = {
        2.0f / data->screenWidth, 0.0f, 0.0f, -1.0f,
        0.0f, -2.0f / data->screenHeight, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f};
    GLuint projLoc = glGetUniformLocation(data->shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection);

    // Set texture uniform
    GLuint texLoc = glGetUniformLocation(data->shaderProgram, "textTexture");
    glUniform1i(texLoc, 0);

    // Draw
    glDrawArrays(GL_TRIANGLES, 0, nverts);

    // Cleanup
    glBindVertexArray(0);
    glUseProgram(0);
}

int FontRenderer::renderCreateImpl(void *userPtr, int width, int height)
{
    FontRenderer::FontData *data = static_cast<FontRenderer::FontData *>(userPtr);
    std::print("Font texture created: {}\n", data->texture);

    // Create texture
    glGenTextures(1, &data->texture);
    glBindTexture(GL_TEXTURE_2D, data->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return 1;
}

int FontRenderer::renderResizeImpl(void *userPtr, int width, int height)
{
    FontRenderer::FontData *data = static_cast<FontRenderer::FontData *>(userPtr);

    // Resize texture
    glBindTexture(GL_TEXTURE_2D, data->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    return 1;
}

void FontRenderer::renderUpdateImpl(void *userPtr, int *rect, const unsigned char *newData)
{
    FontRenderer::FontData *data = static_cast<FontRenderer::FontData *>(userPtr);

    // Update texture region
    glBindTexture(GL_TEXTURE_2D, data->texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, rect[0], rect[1], rect[2] - rect[0], rect[3] - rect[1],
                    GL_RED, GL_UNSIGNED_BYTE, newData);
}

void FontRenderer::renderDeleteImpl(void *userPtr)
{
    FontRenderer::FontData *data = static_cast<FontRenderer::FontData *>(userPtr);

    if (data->texture)
    {
        glDeleteTextures(1, &data->texture);
        data->texture = 0;
    }
}

bool FontRenderer::init(const char *fontPath, int fontSize)
{
    // Create Fontstash context
    FONSparams params;
    memset(&params, 0, sizeof(params));
    params.width = 512;
    params.height = 512;
    params.flags = FONS_ZERO_TOPLEFT;
    params.renderCreate = &FontRenderer::renderCreateImpl;
    params.renderResize = &FontRenderer::renderResizeImpl;
    params.renderUpdate = &FontRenderer::renderUpdateImpl;
    params.renderDraw = &FontRenderer::renderDrawImpl;
    params.renderDelete = &FontRenderer::renderDeleteImpl;
    params.userPtr = data;

    data->fs = fonsCreateInternal(&params);
    if (!data->fs)
    {
        std::cerr << "Failed to create Fontstash context" << std::endl;
        return false;
    }

    // Load font
    data->font = fonsAddFont(data->fs, "default", fontPath);
    if (data->font == FONS_INVALID)
    {
        std::cerr << "Failed to load font: " << fontPath << std::endl;
        return false;
    }

    fonsSetSize(data->fs, fontSize);
    fonsSetColor(data->fs, 0xffffffff); // White, fully opaque
    fonsSetAlign(data->fs, FONS_ALIGN_LEFT | FONS_ALIGN_TOP);

    return true;
}

void FontRenderer::drawText(const std::string &text, float x, float y, const Vec4f &color)
{
    if (!data->fs || data->font == FONS_INVALID)
        return;

    // Convert color to Fontstash format (ABGR)
    unsigned int col = ((unsigned int)(color.w * 255) << 24) |
                       ((unsigned int)(color.z * 255) << 16) |
                       ((unsigned int)(color.y * 255) << 8) |
                       ((unsigned int)(color.x * 255));

    fonsSetColor(data->fs, col);
    fonsDrawText(data->fs, x, y, text.c_str(), nullptr);
}

Vec2f FontRenderer::getTextSize(const std::string &text)
{
    if (!data->fs || data->font == FONS_INVALID)
        return {0, 0};

    float bounds[4];
    fonsTextBounds(data->fs, 0, 0, text.c_str(), nullptr, bounds);

    return {bounds[2] - bounds[0], bounds[3] - bounds[1]};
}

void FontRenderer::setScreenSize(int width, int height)
{
    screenWidth = width;
    screenHeight = height;
    data->screenWidth = width;
    data->screenHeight = height;
}