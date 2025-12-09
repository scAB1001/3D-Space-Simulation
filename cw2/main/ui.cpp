// #include "ui.hpp"
// #include <print>
// #include <fstream>
// #include <vector>

// #define FONTSTASH_IMPLEMENTATION
// #include "fontstash.h"

// // Simple OpenGL fontstash implementation
// struct GLFontStash
// {
//     GLuint texture;
//     int width, height;
// };

// // Vertex structure for quad rendering
// struct QuadVertex
// {
//     float x, y;
//     float r, g, b, a;
// };

// namespace
// {
//     // Fontstash callbacks implementation
//     int renderCreate(void *userPtr, int width, int height)
//     {
//         GLFontStash *gl = static_cast<GLFontStash *>(userPtr);

//         gl->width = width;
//         gl->height = height;

//         glGenTextures(1, &gl->texture);
//         glBindTexture(GL_TEXTURE_2D, gl->texture);
//         glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0,
//                      GL_RED, GL_UNSIGNED_BYTE, nullptr);

//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

//         return 1;
//     }

//     int renderResize(void *userPtr, int width, int height)
//     {
//         GLFontStash *gl = static_cast<GLFontStash *>(userPtr);

//         gl->width = width;
//         gl->height = height;

//         glBindTexture(GL_TEXTURE_2D, gl->texture);
//         glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0,
//                      GL_RED, GL_UNSIGNED_BYTE, nullptr);

//         return 1;
//     }

//     void renderUpdate(void *userPtr, int *rect, const unsigned char *data)
//     {
//         GLFontStash *gl = static_cast<GLFontStash *>(userPtr);

//         int x = rect[0];
//         int y = rect[1];
//         int w = rect[2] - rect[0];
//         int h = rect[3] - rect[1];

//         glBindTexture(GL_TEXTURE_2D, gl->texture);

//         if (data != nullptr)
//         {
//             glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h,
//                             GL_RED, GL_UNSIGNED_BYTE, data);
//         }
//     }

//     void renderDraw(void *userPtr, const float *verts, const float *tcoords,
//                     const unsigned int *colors, int nverts)
//     {
//         GLFontStash *gl = static_cast<GLFontStash *>(userPtr);

//         // Create temporary buffers
//         std::vector<float> vertices;
//         std::vector<float> texcoords;
//         std::vector<float> colorData;

//         vertices.reserve(nverts * 2);
//         texcoords.reserve(nverts * 2);
//         colorData.reserve(nverts * 4);

//         for (int i = 0; i < nverts; ++i)
//         {
//             vertices.push_back(verts[i * 2]);
//             vertices.push_back(verts[i * 2 + 1]);

//             texcoords.push_back(tcoords[i * 2]);
//             texcoords.push_back(tcoords[i * 2 + 1]);

//             unsigned int col = colors[i];
//             colorData.push_back(((col >> 0) & 0xff) / 255.0f);
//             colorData.push_back(((col >> 8) & 0xff) / 255.0f);
//             colorData.push_back(((col >> 16) & 0xff) / 255.0f);
//             colorData.push_back(((col >> 24) & 0xff) / 255.0f);
//         }

//         // Bind texture
//         glBindTexture(GL_TEXTURE_2D, gl->texture);

//         // We'll use immediate mode for simplicity
//         // Enable blending
//         glEnable(GL_BLEND);
//         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//         // Draw triangles
//         glBegin(GL_TRIANGLES);
//         for (int i = 0; i < nverts; ++i)
//         {
//             glTexCoord2f(texcoords[i * 2], texcoords[i * 2 + 1]);
//             glColor4f(colorData[i * 4], colorData[i * 4 + 1],
//                       colorData[i * 4 + 2], colorData[i * 4 + 3]);
//             glVertex2f(vertices[i * 2], vertices[i * 2 + 1]);
//         }
//         glEnd();

//         glDisable(GL_BLEND);
//     }

//     void renderDelete(void *userPtr)
//     {
//         GLFontStash *gl = static_cast<GLFontStash *>(userPtr);
//         if (gl->texture != 0)
//         {
//             glDeleteTextures(1, &gl->texture);
//             gl->texture = 0;
//         }
//         delete gl;
//     }
// }

// UIRenderer::UIRenderer() = default;

// UIRenderer::~UIRenderer()
// {
//     if (mFontContext)
//     {
//         fonsDeleteInternal(mFontContext);
//     }

//     if (mQuadVAO != 0)
//     {
//         glDeleteVertexArrays(1, &mQuadVAO);
//     }

//     if (mQuadVBO != 0)
//     {
//         glDeleteBuffers(1, &mQuadVBO);
//     }

//     if (mQuadShader != 0)
//     {
//         glDeleteProgram(mQuadShader);
//     }

//     if (mFontShader != 0)
//     {
//         glDeleteProgram(mFontShader);
//     }
// }

// bool UIRenderer::initialize(int windowWidth, int windowHeight)
// {
//     mWindowWidth = windowWidth;
//     mWindowHeight = windowHeight;

//     // Setup font system
//     if (!setupFontSystem())
//     {
//         std::print(stderr, "Failed to setup font system\n");
//         return false;
//     }

//     // Setup quad rendering
//     setupQuadRendering();

//     // Compile shaders
//     if (!compileShaders())
//     {
//         std::print(stderr, "Failed to compile UI shaders\n");
//         return false;
//     }

//     return true;
// }

// bool UIRenderer::setupFontSystem()
// {
//     // Create font stash params
//     FONSparams params;
//     memset(&params, 0, sizeof(params));

//     params.width = 512;
//     params.height = 512;
//     params.flags = FONS_ZERO_TOPLEFT;

//     // Set up render callbacks
//     params.renderCreate = renderCreate;
//     params.renderResize = renderResize;
//     params.renderUpdate = renderUpdate;
//     params.renderDraw = renderDraw;
//     params.renderDelete = renderDelete;

//     // Create user data
//     auto *glData = new GLFontStash{0, 512, 512};
//     params.userPtr = glData;

//     // Create context
//     mFontContext = fonsCreateInternal(&params);
//     if (!mFontContext)
//     {
//         delete glData;
//         return false;
//     }

//     // Load font
//     mFontNormal = fonsAddFont(mFontContext, "mono", "assets/cw2/DroidSansMonoDotted.ttf");
//     if (mFontNormal == FONS_INVALID)
//     {
//         std::print(stderr, "Failed to load font DroidSansMonoDotted.ttf\n");
//         return false;
//     }

//     fonsSetFont(mFontContext, mFontNormal);
//     fonsSetSize(mFontContext, mFontHeight);

//     return true;
// }

// void UIRenderer::setupQuadRendering()
// {
//     // Create quad VAO and VBO
//     glGenVertexArrays(1, &mQuadVAO);
//     glGenBuffers(1, &mQuadVBO);

//     glBindVertexArray(mQuadVAO);
//     glBindBuffer(GL_ARRAY_BUFFER, mQuadVBO);

//     // Allocate buffer for 6 vertices (2 triangles)
//     glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(QuadVertex), nullptr, GL_DYNAMIC_DRAW);

//     // Position attribute
//     glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex),
//                           (void *)offsetof(QuadVertex, x));
//     glEnableVertexAttribArray(0);

//     // Color attribute
//     glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(QuadVertex),
//                           (void *)offsetof(QuadVertex, r));
//     glEnableVertexAttribArray(1);

//     glBindVertexArray(0);
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
// }
// GLuint UIRenderer::loadShaderFromFile(const std::string &vertexPath,
//                                       const std::string &fragmentPath)
// {
//     // Read vertex shader
//     std::ifstream vertFile(vertexPath);
//     if (!vertFile.is_open())
//     {
//         std::print(stderr, "Failed to open vertex shader: {}\n", vertexPath);
//         return 0;
//     }

//     std::string vertSource((std::istreambuf_iterator<char>(vertFile)),
//                            std::istreambuf_iterator<char>());
//     vertFile.close();

//     // Read fragment shader
//     std::ifstream fragFile(fragmentPath);
//     if (!fragFile.is_open())
//     {
//         std::print(stderr, "Failed to open fragment shader: {}\n", fragmentPath);
//         return 0;
//     }

//     std::string fragSource((std::istreambuf_iterator<char>(fragFile)),
//                            std::istreambuf_iterator<char>());
//     fragFile.close();

//     // Compile shaders
//     GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
//     GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

//     const char *vertSourceCStr = vertSource.c_str();
//     const char *fragSourceCStr = fragSource.c_str();

//     glShaderSource(vertShader, 1, &vertSourceCStr, nullptr);
//     glShaderSource(fragShader, 1, &fragSourceCStr, nullptr);

//     glCompileShader(vertShader);
//     glCompileShader(fragShader);

//     // Check compilation
//     GLint success;
//     glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
//     if (!success)
//     {
//         char infoLog[512];
//         glGetShaderInfoLog(vertShader, 512, nullptr, infoLog);
//         std::print(stderr, "Vertex shader compilation failed ({}): {}\n",
//                    vertexPath, infoLog);
//         glDeleteShader(vertShader);
//         glDeleteShader(fragShader);
//         return 0;
//     }

//     glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
//     if (!success)
//     {
//         char infoLog[512];
//         glGetShaderInfoLog(fragShader, 512, nullptr, infoLog);
//         std::print(stderr, "Fragment shader compilation failed ({}): {}\n",
//                    fragmentPath, infoLog);
//         glDeleteShader(vertShader);
//         glDeleteShader(fragShader);
//         return 0;
//     }

//     // Link program
//     GLuint program = glCreateProgram();
//     glAttachShader(program, vertShader);
//     glAttachShader(program, fragShader);
//     glLinkProgram(program);

//     glGetProgramiv(program, GL_LINK_STATUS, &success);
//     if (!success)
//     {
//         char infoLog[512];
//         glGetProgramInfoLog(program, 512, nullptr, infoLog);
//         std::print(stderr, "Shader program linking failed: {}\n", infoLog);
//         glDeleteShader(vertShader);
//         glDeleteShader(fragShader);
//         glDeleteProgram(program);
//         return 0;
//     }

//     glDeleteShader(vertShader);
//     glDeleteShader(fragShader);

//     return program;
// }

// bool UIRenderer::compileShaders()
// {
//     // Load quad shader from files
//     mQuadShader = loadShaderFromFile(
//         "assets/cw2/shaders/ui_quad.vert",
//         "assets/cw2/shaders/ui_quad.frag");

//     if (mQuadShader == 0)
//     {
//         std::print(stderr, "Failed to load quad shader\n");
//         return false;
//     }

//     // Load font shader from files
//     mFontShader = loadShaderFromFile(
//         "assets/cw2/shaders/ui_font.vert",
//         "assets/cw2/shaders/ui_font.frag");

//     if (mFontShader == 0)
//     {
//         std::print(stderr, "Failed to load font shader\n");
//         return false;
//     }

//     return true;
// }

// void UIRenderer::updateWindowSize(int width, int height)
// {
//     mWindowWidth = width;
//     mWindowHeight = height;
// }

// void UIRenderer::beginFrame()
// {
//     // Save current OpenGL state
//     glPushAttrib(GL_ENABLE_BIT | GL_VIEWPORT_BIT | GL_COLOR_BUFFER_BIT);

//     // Set up for 2D rendering
//     glDisable(GL_DEPTH_TEST);
//     glDisable(GL_CULL_FACE);
//     glEnable(GL_BLEND);
//     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//     // Set viewport
//     glViewport(0, 0, mWindowWidth, mWindowHeight);
// }

// void UIRenderer::renderText(const std::string &text, float x, float y,
//                             const Vec3f &color, float fontSize)
// {
//     if (!mFontContext || text.empty())
//         return;

//     // Convert color to fontstash format
//     unsigned int fonsColor =
//         ((unsigned int)(color.r * 255.0f)) |
//         ((unsigned int)(color.g * 255.0f) << 8) |
//         ((unsigned int)(color.b * 255.0f) << 16) |
//         ((unsigned int)(255) << 24); // Alpha

//     fonsSetFont(mFontContext, mFontNormal);
//     fonsSetSize(mFontContext, fontSize);
//     fonsSetColor(mFontContext, fonsColor);
//     fonsSetAlign(mFontContext, FONS_ALIGN_LEFT | FONS_ALIGN_TOP);

//     fonsDrawText(mFontContext, x, y, text.c_str(), nullptr);
// }

// void UIRenderer::renderQuad(float x, float y, float width, float height,
//                             const Vec3f &fillColor, float fillAlpha,
//                             const Vec3f &borderColor, float borderAlpha)
// {
//     if (mQuadShader == 0 || mQuadVAO == 0)
//         return;

//     // Use quad shader
//     glUseProgram(mQuadShader);

//     // Define quad vertices (2 triangles)
//     QuadVertex vertices[6] = {
//         // Triangle 1
//         {x, y, fillColor.r, fillColor.g, fillColor.b, fillAlpha},
//         {x + width, y, fillColor.r, fillColor.g, fillColor.b, fillAlpha},
//         {x + width, y + height, fillColor.r, fillColor.g, fillColor.b, fillAlpha},

//         // Triangle 2
//         {x, y, fillColor.r, fillColor.g, fillColor.b, fillAlpha},
//         {x + width, y + height, fillColor.r, fillColor.g, fillColor.b, fillAlpha},
//         {x, y + height, fillColor.r, fillColor.g, fillColor.b, fillAlpha}};


//     // Set screen size uniform
//     GLint screenSizeLoc = glGetUniformLocation(mQuadShader, "uScreenSize");
//     if(screenSizeLoc != -1)
//     {
//         glUniform2f(screenSizeLoc, (float)mWindowWidth, (float)mWindowHeight);
//     }

//     // Upload vertex data
//     glBindVertexArray(mQuadVAO);
//     glBindBuffer(GL_ARRAY_BUFFER, mQuadVBO);
//     glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

//     // Draw fill
//     glDrawArrays(GL_TRIANGLES, 0, 6);

//     // Draw border (using line loop)
//     QuadVertex borderVertices[4] = {
//         {x, y, borderColor.r, borderColor.g, borderColor.b, borderAlpha},
//         {x + width, y, borderColor.r, borderColor.g, borderColor.b, borderAlpha},
//         {x + width, y + height, borderColor.r, borderColor.g, borderColor.b, borderAlpha},
//         {x, y + height, borderColor.r, borderColor.g, borderColor.b, borderAlpha}};

//     // Upload border vertices
//     glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(QuadVertex), borderVertices);

//     // Draw border as line loop
//     glDrawArrays(GL_LINE_LOOP, 0, 4);

//     // Cleanup
//     glBindVertexArray(0);
//     glUseProgram(0);
// }

// void UIRenderer::renderButton(Button &button)
// {
//     if (!button.isVisible)
//         return;

//     // Choose colors based on state
//     Button::Colors colors;
//     switch (button.state)
//     {
//     case ButtonState::Hover:
//         colors = button.colorsHover;
//         break;
//     case ButtonState::Pressed:
//         colors = button.colorsPressed;
//         break;
//     default:
//         colors = button.colorsNormal;
//         break;
//     }

//     // Render button quad
//     renderQuad(button.position.x, button.position.y,
//                button.size.x, button.size.y,
//                colors.fill, colors.alpha,
//                colors.border, colors.alpha * 1.2f); // Slightly darker border

//     // Calculate text position (centered)
//     float textX = button.position.x + button.size.x / 2.0f;
//     float textY = button.position.y + button.size.y / 2.0f - mFontHeight / 3.0f;

//     // Render button text
//     renderText(button.label, textX, textY, colors.text, mFontHeight);
// }

// UIRenderer::Button *UIRenderer::createButton(const std::string &label, Vec2f position,
//                                              Vec2f size, std::function<void()> onClick)
// {
//     static std::vector<Button> buttons;

//     Button newButton;
//     newButton.label = label;
//     newButton.position = position;
//     newButton.size = size;
//     newButton.onClick = onClick;

//     // Set default colors
//     newButton.colorsNormal = {
//         .fill = {0.2f, 0.2f, 0.2f},
//         .border = {0.4f, 0.4f, 0.4f},
//         .text = {1.0f, 1.0f, 1.0f},
//         .alpha = 0.8f};

//     newButton.colorsHover = {
//         .fill = {0.3f, 0.3f, 0.3f},
//         .border = {0.5f, 0.5f, 0.5f},
//         .text = {1.0f, 1.0f, 1.0f},
//         .alpha = 0.9f};

//     newButton.colorsPressed = {
//         .fill = {0.4f, 0.2f, 0.2f},
//         .border = {0.6f, 0.4f, 0.4f},
//         .text = {1.0f, 1.0f, 1.0f},
//         .alpha = 1.0f};

//     buttons.push_back(newButton);
//     return &buttons.back();
// }

// void UIRenderer::updateButtonState(Button &button, Vec2f mousePos, bool mousePressed)
// {
//     if (!button.isVisible)
//         return;

//     bool isInside = button.contains(mousePos);

//     if (isInside)
//     {
//         if (mousePressed)
//         {
//             button.state = ButtonState::Pressed;
//         }
//         else
//         {
//             button.state = ButtonState::Hover;
//         }
//     }
//     else
//     {
//         button.state = ButtonState::Normal;
//     }
// }

// void UIRenderer::endFrame()
// {
//     // Restore OpenGL state
//     glPopAttrib();
// }