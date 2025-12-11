#include "renderer.hpp"
#include "../vmlib/mat44.hpp"
#include "../vmlib/mat33.hpp"
#include "../vmlib/vec3.hpp"
#include "../support/error.hpp"

#include "space_vehicle.hpp"
#include "config.hpp"
#include "state.hpp"


#include <glad/glad.h>
#include <print>



void setCW2LightingUniforms(State_& state,
                            const Mat44f& modelVehicle,
                            const Vec3f& cameraPos);

void computeVehicleLights(State_& state,
                          const Mat44f& modelVehicle);

void globalGLSetup()
{
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glClearDepthf(1.f);
    glClearColor(0.2f, 0.2f, 0.2f, 0.f);
}

void beginFrame()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void resetBindings()
{
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void endFrame()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);

    resetBindings();
}

// ------------------------------------------------------------
// TASK 1.6 — Compute point light positions attached to vehicle
// ------------------------------------------------------------
void computeVehicleLights(State_& state, const Mat44f& modelVehicle)
{
    float bodyHeight = 5.0f;
    float finY = 0.3f * bodyHeight;   // = 1.5
    float finDist = 3.2f;             // Position lights far outside fins

    float angles[3] = { 0.f, 2.0944f, 4.1888f }; // 0°, 120°, 240°

    for (int i = 0; i < 3; i++)
    {
        float a = angles[i];

        Vec3f localPos {
            finDist * std::cos(a),
            finY,
            finDist * std::sin(a)
        };

        // Convert Vec3f -> Vec4f manually (your math lib requires this)
        Vec4f p4 { localPos.x, localPos.y, localPos.z, 1.f };

        Vec4f wp = modelVehicle * p4;
        state.pointLights[i].position = Vec3f{ wp.x, wp.y, wp.z };
    }
}


// ------------------------------------------------------------
// TASK 1.6 — Upload ALL lighting uniforms used in u.frag
// ------------------------------------------------------------
void setCW2LightingUniforms(
    State_& state,
    const Mat44f& modelVehicle,
    const Vec3f& cameraPos)
{
    computeVehicleLights(state, modelVehicle);

    // Shader is already bound in main.cpp
    // So all glUniform calls apply to unifiedProg

    // Directional light toggle
    glUniform1i(glGetUniformLocation(state.prog->programId(), "uDirLightEnabled"),
                state.dirLightEnabled);

    // Directional light values (from Config)
    Vec3f L = Config::Rendering::kLightDir;
    Vec3f C = Config::Rendering::kLightDiffuse;

    glUniform3fv(glGetUniformLocation(state.prog->programId(), "uDirLightDir"), 1, &L.x);
    glUniform3fv(glGetUniformLocation(state.prog->programId(), "uDirLightColor"), 1, &C.x);

    // Camera position
    glUniform3fv(glGetUniformLocation(state.prog->programId(), "uCameraPos"), 1, &cameraPos.x);

    // Point lights
    for (int i = 0; i < 3; i++)
    {
        std::string base = "uPointLight[" + std::to_string(i) + "]";

        glUniform1i(
            glGetUniformLocation(state.prog->programId(), (base + ".enabled").c_str()),
            state.pointLights[i].enabled
        );

        glUniform3fv(
            glGetUniformLocation(state.prog->programId(), (base + ".position").c_str()),
            1,
            &state.pointLights[i].position.x
        );

        glUniform3fv(
            glGetUniformLocation(state.prog->programId(), (base + ".color").c_str()),
            1,
            &state.pointLights[i].color.x
        );
    }
}

void setLightingUniforms(
    const Vec3f &lightDir,
    const Vec3f &lightDiffuse,
    const Vec3f &sceneAmbient)
{
    Vec3f normalizedLightDir = normalize(lightDir);

    glUniform3fv(2, 1, &normalizedLightDir.x); // uLightDir
    glUniform3fv(3, 1, &lightDiffuse.x);       // uLightDiffuse
    glUniform3fv(4, 1, &sceneAmbient.x);       // uSceneAmbient
}


// ------------------------------------------------------------
// DRAWING HELPERS (unchanged from Tasks 1.1–1.5)
// ------------------------------------------------------------
void drawMesh(
    GLuint vao,
    GLsizei vertexCount,
    bool hasIndices,
    GLsizei indexCount,
    int materialType,
    GLuint texture,
    const Mat44f &projCameraWorld,
    const Mat33f &normalMatrix,
    const Mat44f &modelMatrix)
{
    // ----- MATERIAL TYPE -----
    glUniform1i(10, materialType);

    // ----- TRANSFORM MATRICES -----
    glUniformMatrix4fv(0, 1, GL_TRUE, projCameraWorld.v); // uProjCameraWorld
    glUniformMatrix3fv(1, 1, GL_TRUE, normalMatrix.v);    // uNormalMatrix

    // ----- uModel (required for point lights) -----
    GLint currentProg = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProg);

    GLint locModel = glGetUniformLocation(currentProg, "uModel");
    if (locModel >= 0)
    {
        glUniformMatrix4fv(locModel, 1, GL_TRUE, modelMatrix.v);
    }

    // ----- TEXTURE BINDING -----
    if (materialType == 1 && texture != 0)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // ----- DRAW CALL -----
    glBindVertexArray(vao);

    if (hasIndices && indexCount > 0)
    {
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }
}




void drawLandingPads(
    const std::vector<LandingPad> &pads,
    const Mat44f &projView)
{
    // Material type = coloured (0)
    glUniform1i(10, 0);

    // No texture
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindVertexArray(pads[0].vao);

    // Query active program for uniform locations
    GLint currentProg = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProg);

    const GLint locProjCameraWorld = 0;  // explicit in u.vert
    const GLint locNormalMatrix    = 1;  // explicit in u.vert

    // uModel: NOT explicit — must be looked up dynamically
    const GLint locModel = glGetUniformLocation(currentProg, "uModel");

    for (const auto &pad : pads)
    {
        Mat44f modelMatrix      = pad.transform;
        Mat44f projCameraWorld  = projView * modelMatrix;
        Mat33f normalMatrix     = make_uniform_normal(modelMatrix);

        // Upload matrices using correct locations
        glUniformMatrix4fv(locProjCameraWorld, 1, GL_TRUE, projCameraWorld.v);
        glUniformMatrix3fv(locNormalMatrix,    1, GL_TRUE, normalMatrix.v);

        if (locModel >= 0)
            glUniformMatrix4fv(locModel, 1, GL_TRUE, modelMatrix.v);

        // Draw the landing pad mesh
        glDrawArrays(GL_TRIANGLES, 0, pad.vertexCount);
    }
}



void drawTerrain(
    GLuint vao,
    GLsizei vertexCount,
    GLuint texture,
    const Mat44f &projCameraWorld,
    const Mat33f &normalMatrix,
    const Mat44f &modelMatrix)   // NEW
{
    drawMesh(
        vao,
        vertexCount,
        false,         // no indices
        0,
        1,             // materialType = textured
        texture,
        projCameraWorld,
        normalMatrix,
        modelMatrix     // NEW
    );
}


void drawColoredObject(
    GLuint vao,
    GLsizei vertexCount,
    GLsizei indexCount,
    const Mat44f &projCameraWorld,
    const Mat33f &normalMatrix,
    const Mat44f &modelMatrix)   // NEW
{
    drawMesh(
        vao,
        vertexCount,
        true,   // has indices
        indexCount,
        0,      // materialType = coloured
        0,      // no texture
        projCameraWorld,
        normalMatrix,
        modelMatrix
    );
}
