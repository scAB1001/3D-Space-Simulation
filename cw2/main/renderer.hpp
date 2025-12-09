#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <glad/glad.h>
#include "../vmlib/mat44.hpp"
#include "../vmlib/mat33.hpp"
#include "../vmlib/vec3.hpp"
#include "landing_pad.hpp"

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
    const Mat33f &normalMatrix);

void drawLandingPads(
    const std::vector<LandingPad> &pads,
    const Mat44f &projView);

void drawTerrain(
    GLuint vao,
    GLsizei vertexCount,
    GLuint texture,
    const Mat44f &projCameraWorld,
    const Mat33f &normalMatrix);

void drawColoredObject(
    GLuint vao,
    GLsizei vertexCount,
    GLsizei indexCount,
    const Mat44f &projCameraWorld,
    const Mat33f &normalMatrix);

#endif // RENDERER_HPP