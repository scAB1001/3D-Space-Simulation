#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <glad/glad.h>
#include "../vmlib/mat44.hpp"
#include "../vmlib/mat33.hpp"
#include "../vmlib/vec3.hpp"


// Scene setup (clearing, state)
void globalGLSetup();

void beginFrame();
void endFrame();

// Lighting setup (call once per frame)
void setLightingUniforms(
    const Vec3f &lightDir,
    const Vec3f &lightDiffuse,
    const Vec3f &sceneAmbient);

// Object rendering functions (only drawing + uniform setting)
void drawMesh(
    GLuint vao,
    GLsizei vertexCount,
    bool hasIndices,
    GLsizei indexCount,
    int materialType,
    GLuint texture,
    const Mat44f &projCameraWorld,
    const Mat33f &normalMatrix);

void drawLandingPad(
    GLuint vao,
    GLsizei vertexCount,
    const Mat44f &projCameraWorld,
    const Mat33f &normalMatrix);

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