#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <glad/glad.h>

#include "../vmlib/mat44.hpp"
#include "../vmlib/mat33.hpp"
#include "../vmlib/vec3.hpp"
#include "../support/error.hpp"

#include "landing_pad.hpp"
#include "space_vehicle.hpp"
#include "config.hpp"
#include "state.hpp"
#include "space_vehicle.hpp"
#include "config.hpp"
#include "state.hpp"

namespace RendererInternal {
    inline GLuint currentShaderProgram = 0;

    inline void bindShader(GLuint program)
    {
        if (program != currentShaderProgram)
        {
            glUseProgram(program);
            currentShaderProgram = program;
        }
    }
}

using RendererInternal::bindShader;


// Scene setup and frame management
void globalGLSetup();
void beginFrame();
void resetBindings();
void endFrame();
void cleanup(State_ &state, GLuint terrainVao, GLuint vehicleVao, GLuint terrainTexture);

// Setting light uniforms
void setDirectionalLightUniforms(
    const Vec3f &lightDir,
    const Vec3f &lightDiffuse,
    const Vec3f &sceneAmbient);
void computeVehicleLights(State_ &state, const Mat44f &modelVehicle);
void setPointLightUniforms(State_ &state);
void setAllLightingUniforms(State_ &state,
                            const Mat44f &modelVehicle,
                            const Camera &cam);

struct RenderContext
{
    GLuint parlahtiVao;
    std::size_t parlahtiVertexCount;
    GLuint parlahtiTexture;
    const std::vector<LandingPad> &landingPads;
    GLuint vehicleVao;
    std::size_t vehicleVertexCount;
    std::size_t vehicleIndexCount;
    GLuint unifiedProg;
};

void renderView(
    Camera &cam,
    float vpWidth,
    float vpHeight,
    const Mat44f &model2world_vehicle,
    ParticleSystem &particles,
    const RenderContext &ctx);

void renderScreen(
    State_ &state,
    Camera &cam,
    GLint xViewport,
    GLsizei vpWidth,
    GLsizei vpHeight,
    const Mat44f &model2world_vehicle,
    ParticleSystem &particles,
    const RenderContext &ctx);

void renderSingleOrSplitScreen(
    State_ &state,
    const Mat44f &model2world_vehicle,
    GLsizei fbwidth,
    GLsizei fbheight,
    ParticleSystem &particles,
    const RenderContext &ctx);

// Drawing helpers
void drawMesh(
    GLuint vao,
    GLsizei vertexCount,
    bool hasIndices,
    GLsizei indexCount,
    int materialType,
    GLuint texture,
    const Mat44f &projCameraWorld,
    const Mat33f &normalMatrix,
    const Mat44f &modelMatrix);

void drawTerrain(
    GLuint vao,
    GLsizei vertexCount,
    GLuint texture,
    const Mat44f &projCameraWorld,
    const Mat33f &normalMatrix,
    const Mat44f &modelMatrix);

void drawLandingPads(
    const std::vector<LandingPad> &pads,
    const Mat44f &projView);

void drawObject(
    GLuint vao,
    GLsizei vertexCount,
    GLsizei indexCount,
    const Mat44f &projCameraWorld,
    const Mat33f &normalMatrix,
    const Mat44f &modelMatrix);

#endif // RENDERER_HPP