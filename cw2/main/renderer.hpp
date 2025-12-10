#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <glad/glad.h>
#include "../vmlib/mat44.hpp"
#include "../vmlib/mat33.hpp"
#include "../vmlib/vec3.hpp"
#include "landing_pad.hpp"
#include "state.hpp"   // <-- MUST BE HERE

void computeVehicleLights(State_& state, const Mat44f& modelVehicle);

void setCW2LightingUniforms(State_& state,
                            const Mat44f& modelVehicle,
                            const Vec3f& cameraPos);

// Scene setup and frame management
void globalGLSetup();
void beginFrame();
void resetBindings();
void endFrame();

// Lighting setup (call once per frame)
void setLightingUniforms(
    const Vec3f &lightDir,
    const Vec3f &lightDiffuse,
    const Vec3f &sceneAmbient);

void drawMesh(
    GLuint vao,
    GLsizei vertexCount,
    bool hasIndices,
    GLsizei indexCount,
    int materialType,
    GLuint texture,
    const Mat44f &projCameraWorld,
    const Mat33f &normalMatrix,
    const Mat44f &modelMatrix);   // NEW

void drawLandingPads(
    const std::vector<LandingPad> &pads,
    const Mat44f &projView);

void drawTerrain(
    GLuint vao,
    GLsizei vertexCount,
    GLuint texture,
    const Mat44f &projCameraWorld,
    const Mat33f &normalMatrix,
    const Mat44f &modelMatrix);   // NEW

void drawColoredObject(
    GLuint vao,
    GLsizei vertexCount,
    GLsizei indexCount,
    const Mat44f &projCameraWorld,
    const Mat33f &normalMatrix,
    const Mat44f &modelMatrix);   // NEW

#endif // RENDERER_HPP