#include "renderer.hpp"
#include <print>

void globalGLSetup()
{
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearDepthf(1.f);
    // dark gray - values between 0.0 and 1.0
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
}

void beginFrame()
{
    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set temporary render state (as in your original code)
    // glDisable(GL_CULL_FACE); // Temporary

    // Optional: Wireframe mode for debugging
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void endFrame()
{
    // Restore default render state
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);

    // Cleanup: unbind VAO and program
    glBindVertexArray(0);
    glUseProgram(0);
}

void setLightingUniforms(
    const Vec3f &lightDir,
    const Vec3f &lightDiffuse,
    const Vec3f &sceneAmbient)
{
    // Normalize light direction
    Vec3f normalizedLightDir = normalize(lightDir);

    // Set uniforms (assuming your shader uses locations 2, 3, 4)
    glUniform3fv(2, 1, &normalizedLightDir.x); // uLightDir
    glUniform3fv(3, 1, &lightDiffuse.x);       // uLightDiffuse
    glUniform3fv(4, 1, &sceneAmbient.x);       // uSceneAmbient

    // TODO: Remove later. Debug output
    // std::print("Lighting set: dir=({:.2f},{:.2f},{:.2f})\n",
    //            normalizedLightDir.x, normalizedLightDir.y, normalizedLightDir.z);
}

void drawMesh(
    GLuint vao,
    GLsizei vertexCount,
    bool hasIndices,
    GLsizei indexCount,
    int materialType,
    GLuint texture,
    const Mat44f &projCameraWorld,
    const Mat33f &normalMatrix)
{
    // Set material type uniform
    glUniform1i(10, materialType); // uMaterialType

    // Set transformation matrices
    glUniformMatrix4fv(0, 1, GL_TRUE, projCameraWorld.v); // uProjCameraWorld
    glUniformMatrix3fv(1, 1, GL_TRUE, normalMatrix.v);    // uNormalMatrix

    // Bind texture if material is textured
    if (materialType == 1 && texture != 0)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, 0); // No texture
    }

    // Bind VAO and draw
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

// Convenience wrappers for specific object types
void drawLandingPad(
    GLuint vao,
    GLsizei vertexCount,
    const Mat44f &projCameraWorld,
    const Mat33f &normalMatrix)
{
    drawMesh(
        vao,
        vertexCount,
        false, // landing pads don't use indices
        0,     // no index count
        0,     // materialType = 0 (colored)
        0,     // no texture
        projCameraWorld,
        normalMatrix);
}

void drawTerrain(
    GLuint vao,
    GLsizei vertexCount,
    GLuint texture,
    const Mat44f &projCameraWorld,
    const Mat33f &normalMatrix)
{
    drawMesh(
        vao,
        vertexCount,
        false, // hasIndices,
        0,     // indexCount,
        1,     // materialType,
        texture,
        projCameraWorld,
        normalMatrix);
}

void drawColoredObject(
    GLuint vao,
    GLsizei vertexCount,
    GLsizei indexCount,
    const Mat44f &projCameraWorld,
    const Mat33f &normalMatrix)
{
    drawMesh(
        vao,
        vertexCount,
        true,
        indexCount,
        0, // materialType = 0 (colored)
        0, // no texture
        projCameraWorld,
        normalMatrix);
}