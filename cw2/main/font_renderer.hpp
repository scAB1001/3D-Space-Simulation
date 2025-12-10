#ifndef FONT_RENDERER_HPP
#define FONT_RENDERER_HPP

#include <string>
#include "../vmlib/vec2.hpp"
#include "../vmlib/vec4.hpp"

// Forward declaration to avoid including fontstash.h in header
struct FONScontext;

class FontRenderer
{
public:
    FontRenderer();
    ~FontRenderer();

    // Initialize with font file
    bool init(const char *fontPath, int fontSize = 16);

    // Draw text at screen position (in pixels)
    void drawText(const std::string &text, float x, float y, const Vec4f &color = {1.f, 1.f, 1.f, 1.f});

    // Get text dimensions
    Vec2f getTextSize(const std::string &text);

    // Set screen dimensions (call when window resizes)
    void setScreenSize(int width, int height);

private:
    // Forward declare FontData - full definition will be in .cpp
    struct FontData;
    FontData *data;
    int screenWidth, screenHeight;

    // Static callback helpers (these need access to FontData)
    static void renderDrawImpl(void *userPtr, const float *verts, const float *tcoords, const unsigned int *colors, int nverts);
    static int renderCreateImpl(void *userPtr, int width, int height);
    static int renderResizeImpl(void *userPtr, int width, int height);
    static void renderUpdateImpl(void *userPtr, int *rect, const unsigned char *data);
    static void renderDeleteImpl(void *userPtr);
};

#endif // FONT_RENDERER_HPP