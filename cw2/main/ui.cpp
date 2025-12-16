#include "ui.hpp"

#include <glad/glad.h>
#include <string>

#include "../vmlib/mat44.hpp"
#include "../vmlib/mat33.hpp"
#include "../vmlib/vec3.hpp"

#include "renderer.hpp"   // bindShader, kIdentity matrices
#include "config.hpp"

// ------------------------------------------------------------
// Internal UI helpers
// ------------------------------------------------------------

struct UIVertex
{
    Vec3f pos;
    Vec3f color;
};

static GLuint uiVao = 0;
static GLuint uiVbo = 0;

static void initUIQuad()
{
    if (uiVao != 0)
        return;

    UIVertex verts[6] = {
        {{0,0,0},{1,1,1}}, {{1,0,0},{1,1,1}}, {{1,1,0},{1,1,1}},
        {{0,0,0},{1,1,1}}, {{1,1,0},{1,1,1}}, {{0,1,0},{1,1,1}},
    };

    glGenVertexArrays(1, &uiVao);
    glGenBuffers(1, &uiVbo);

    glBindVertexArray(uiVao);
    glBindBuffer(GL_ARRAY_BUFFER, uiVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    // iPosition
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE,
        sizeof(UIVertex), (void*)0
    );

    // iColor
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE,
        sizeof(UIVertex), (void*)sizeof(Vec3f)
    );

    glBindVertexArray(0);
}

// ------------------------------------------------------------
// OpenGL state for UI
// ------------------------------------------------------------

static void beginUI(float fbWidth, float fbHeight)
{
    glViewport(0, 0, (GLsizei)fbWidth, (GLsizei)fbHeight);

    glDisable(GL_FRAMEBUFFER_SRGB);   // ← REQUIRED
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}



static void endUI()
{
    glDisable(GL_BLEND);

    glEnable(GL_FRAMEBUFFER_SRGB);    // ← RESTORE
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}


// ------------------------------------------------------------
// Orthographic projection (manual)
// ------------------------------------------------------------

static Mat44f make_ortho_ui(
    float left, float right,
    float bottom, float top,
    float nearZ, float farZ)
{
    Mat44f m{};

    m.v[0]  =  2.f / (right - left);
    m.v[5]  =  2.f / (top - bottom);
    m.v[10] = -2.f / (farZ - nearZ);
    m.v[12] = -(right + left) / (right - left);
    m.v[13] = -(top + bottom) / (top - bottom);
    m.v[14] = -(farZ + nearZ) / (farZ - nearZ);
    m.v[15] =  1.f;

    return m;
}

// ------------------------------------------------------------
// Rectangle drawing
// ------------------------------------------------------------

static void drawRect(
    float x, float y, float w, float h,
    const Mat44f& ortho,
    GLuint shader)
{
    Mat44f model =
        make_translation(Vec3f{x, y, 0.f}) *
        make_scaling(w, h, 1.f);

    Mat44f mvp = ortho * model;

    glUseProgram(shader);

    GLint loc = glGetUniformLocation(shader, "uMVP");
    glUniformMatrix4fv(loc, 1, GL_TRUE, mvp.v);

    glBindVertexArray(uiVao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// ------------------------------------------------------------
// Public entry point (Task 1.11)
// ------------------------------------------------------------

void renderUI(State_& state,
              float fbWidth,
              float fbHeight,
              GLuint uiShader)
{
    initUIQuad();
    beginUI(fbWidth, fbHeight);

    Mat44f ortho = make_ortho_ui(
        0.f, fbWidth,
        fbHeight, 0.f,
        -1.f, 1.f
    );

    float btnW = 160.f;
    float btnH = 50.f;
    float gap  = 20.f;

    float centerX = fbWidth * 0.5f;
    float y = fbHeight - btnH - 20.f;

    float launchX = centerX - btnW - gap * 0.5f;
    float resetX  = centerX + gap * 0.5f;

    drawRect(launchX, y, btnW, btnH, ortho, uiShader);
    drawRect(resetX,  y, btnW, btnH, ortho, uiShader);

    endUI();
}

