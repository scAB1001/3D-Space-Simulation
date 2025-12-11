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

void resetBindings()
{
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void endFrame()
{
    // Restore default render state
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);

    // Cleanup: unbind VAO and program
    resetBindings();
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
// void drawLandingPads(
//     GLuint vao,
//     GLsizei vertexCount,
//     const Mat44f &projCameraWorld,
//     const Mat33f &normalMatrix)
// {
//     // Set material type uniform
//     glUniform1i(10, 0); // uMaterialType

//     // Set transformation matrices
//     glUniformMatrix4fv(0, 1, GL_TRUE, projCameraWorld.v);
//     glUniformMatrix3fv(1, 1, GL_TRUE, normalMatrix.v);

//     // No texture
//     glBindTexture(GL_TEXTURE_2D, 0);

//     // With texture
//     // glUniform1i(10, 1);
//     // glActiveTexture(GL_TEXTURE0);
//     // glBindTexture(GL_TEXTURE_2D, 1);

//     // Bind VAO and draw
//     glBindVertexArray(vao);

//     glDrawArrays(GL_TRIANGLES, 0, vertexCount); // Pad
// }

void drawLandingPads(
    const std::vector<LandingPad> &pads,
    const Mat44f &projView)
{
    glUniform1i(10, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Bind VAO once (so that all pads share it)
    // this works by because all pads use the same (shared) VAO
    glBindVertexArray(pads[0].vao);

    // Draw each pad by updating its transform, normal matrix and then drawing
    for (const auto &pad : pads)
    {
        Mat44f projCameraWorld = projView * pad.transform;
        Mat33f normalMatrix = make_uniform_normal(pad.transform);

        glUniformMatrix4fv(0, 1, GL_TRUE, projCameraWorld.v);
        glUniformMatrix3fv(1, 1, GL_TRUE, normalMatrix.v);

        glDrawArrays(GL_TRIANGLES, 0, pad.vertexCount);
    }
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