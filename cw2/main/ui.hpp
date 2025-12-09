// #ifndef UI_HPP
// #define UI_HPP

// #include <glad/glad.h>
// #include <GLFW/glfw3.h>
// #include "../vmlib/vec2.hpp"
// #include "../vmlib/vec3.hpp"
// #include <string>
// #include <functional>

// // Forward declaration for fontstash context
// struct FONScontext;

// class UIRenderer
// {
// public:
//     // Button states
//     enum class ButtonState
//     {
//         Normal,
//         Hover,
//         Pressed
//     };

//     // Button structure
//     struct Button
//     {
//         std::string label;
//         Vec2f position; // Bottom-left corner in screen coordinates (0,0 bottom-left)
//         Vec2f size;     // Width, height
//         ButtonState state = ButtonState::Normal;
//         bool isVisible = true;
//         std::function<void()> onClick;

//         // Colors for different states
//         struct Colors
//         {
//             Vec3f fill;
//             Vec3f border;
//             Vec3f text;
//             float alpha;
//         } colorsNormal, colorsHover, colorsPressed;

//         // Check if point is inside button
//         bool contains(Vec2f point) const
//         {
//             return point.x >= position.x &&
//                    point.x <= position.x + size.x &&
//                    point.y >= position.y &&
//                    point.y <= position.y + size.y;
//         }
//     };

// public:
//     UIRenderer();
//     ~UIRenderer();

//     // Initialize UI system
//     bool initialize(int windowWidth, int windowHeight);

//     // Update window size
//     void updateWindowSize(int width, int height);

//     // Rendering
//     void beginFrame();
//     void renderText(const std::string &text, float x, float y,
//                     const Vec3f &color = {1.f, 1.f, 1.f},
//                     float fontSize = 16.f);
//     void renderButton(Button &button);
//     void endFrame();

//     // UI Management
//     Button *createButton(const std::string &label, Vec2f position, Vec2f size,
//                          std::function<void()> onClick);
//     void updateButtonState(Button &button, Vec2f mousePos, bool mousePressed);

//     // Get current window size
//     Vec2f getWindowSize() const { return Vec2f{static_cast<float>(mWindowWidth),
//                                                static_cast<float>(mWindowHeight)}; }

// private:
//     // Font system
//     FONScontext *mFontContext = nullptr;

//     // OpenGL resources
//     GLuint mFontTexture = 0;
//     GLuint mFontShader = 0;
//     GLuint mQuadShader = 0;
//     GLuint mQuadVAO = 0;
//     GLuint mQuadVBO = 0;

//     // Window dimensions
//     int mWindowWidth = 0;
//     int mWindowHeight = 0;

//     // Font metrics
//     float mFontHeight = 20.f;
//     int mFontNormal = -1;

// private:
//     // Helper methods
//     GLuint loadShaderFromFile(const std::string &vertexPath,
//                               const std::string &fragmentPath);
//     bool compileShaders();
//     bool setupFontSystem();
//     void setupQuadRendering();
//     void renderQuad(float x, float y, float width, float height,
//                     const Vec3f &fillColor, float fillAlpha,
//                     const Vec3f &borderColor, float borderAlpha);

//     // Fontstash callbacks
//     static int renderCreate(void *userPtr, int width, int height);
//     static int renderResize(void *userPtr, int width, int height);
//     static void renderUpdate(void *userPtr, int *rect, const unsigned char *data);
//     static void renderDraw(void *userPtr, const float *verts, const float *tcoords,
//                            const unsigned int *colors, int nverts);
//     static void renderDelete(void *userPtr);
// };

// #endif // UI_HPP