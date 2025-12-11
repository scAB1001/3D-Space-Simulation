#include "renderer.hpp"

// Scene setup and frame management
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

void cleanup(State_ &state, GLuint terrainVao, GLuint vehicleVao, GLuint terrainTexture)
{
    state.prog = nullptr;

	glDeleteVertexArrays(1, &terrainVao);
	glDeleteVertexArrays(1, &vehicleVao);

	if (terrainTexture != 0)
		glDeleteTextures(1, &terrainTexture);

    LandingPad::cleanup();
}

// Setting light uniforms
void setDirectionalLightUniforms(
    const Vec3f &lightDir,
    const Vec3f &lightDiffuse,
    const Vec3f &sceneAmbient)
{
    Vec3f L = normalize(lightDir);

    // Query the current program
    GLint prog;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prog);

    glUniform3fv(glGetUniformLocation(prog, "uLightDir"), 1, &L.x);
    glUniform3fv(glGetUniformLocation(prog, "uLightDiffuse"), 1, &lightDiffuse.x);
    glUniform3fv(glGetUniformLocation(prog, "uSceneAmbient"), 1, &sceneAmbient.x);
}

void computeVehicleLights(State_ &state, const Mat44f &modelVehicle)
{
    float bodyHeight = 5.0f;
    float finY = 0.3f * bodyHeight; // = 1.5
    float finDist = 3.2f;           // Position lights far outside fins

    float angles[3] = {0.f, 2.0944f, 4.1888f}; // 0°, 120°, 240°

    for (int i = 0; i < 3; i++)
    {
        float a = angles[i];

        Vec3f localPos{
            finDist * std::cos(a),
            finY,
            finDist * std::sin(a)};

        Vec4f p4 { localPos.x, localPos.y, localPos.z, 1.f };

        Vec4f wp = modelVehicle * p4;
        state.pointLights[i].position = Vec3f{wp.x, wp.y, wp.z};
    }
}

void setPointLightUniforms(State_ &state)
{
    GLuint progId = state.prog->programId();

    // Point light colors
    glUniform3fv(glGetUniformLocation(progId, "uPointPos[0]"), 1, &state.pointLights[0].position.x);
    glUniform3fv(glGetUniformLocation(progId, "uPointPos[1]"), 1, &state.pointLights[1].position.x);
    glUniform3fv(glGetUniformLocation(progId, "uPointPos[2]"), 1, &state.pointLights[2].position.x);

    // Point light colors
    glUniform3fv(glGetUniformLocation(progId, "uPointColor[0]"), 1, &state.pointLights[0].color.x);
    glUniform3fv(glGetUniformLocation(progId, "uPointColor[1]"), 1, &state.pointLights[1].color.x);
    glUniform3fv(glGetUniformLocation(progId, "uPointColor[2]"), 1, &state.pointLights[2].color.x);

    // Point light enabled flags
    GLint enabled[3] = {
        state.pointLights[0].enabled ? 1 : 0,
        state.pointLights[1].enabled ? 1 : 0,
        state.pointLights[2].enabled ? 1 : 0
    };
    glUniform1iv(glGetUniformLocation(progId, "uPointEnabled[0]"), 3, enabled);
}

void setAllLightingUniforms(
    State_ &state,
    const Mat44f &modelVehicle)
{
    // Directional light uniforms
    setDirectionalLightUniforms(
        Config::Rendering::kLightDir,
        Config::Rendering::kLightDiffuse,
        Config::Rendering::kSceneAmbient);

    // Point light positions based on vehicle model matrix
    computeVehicleLights(state, modelVehicle);

    // Point light uniforms
    setPointLightUniforms(state);

    // Directional light enabled flag
    GLuint progId = state.prog->programId();
    glUniform1i(glGetUniformLocation(progId, "uGlobalDirLightEnabled"), state.globalDirLightEnabled ? 1 : 0);

    // Camera position
    Vec3f camPos = state.camera.getPosition();
    glUniform3fv(glGetUniformLocation(progId, "uCameraPos"), 1, &camPos.x);
}


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
    const Mat44f &modelMatrix)
{
    glUniform1i(10, materialType);
    glUniformMatrix4fv(0, 1, GL_TRUE, projCameraWorld.v);
    glUniformMatrix3fv(1, 1, GL_TRUE, normalMatrix.v);

    // ----- uModel (for point lights) -----
    GLint currentProg = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProg);

    GLint locModel = glGetUniformLocation(currentProg, "uModel");
    if (locModel >= 0)
    {
        glUniformMatrix4fv(locModel, 1, GL_TRUE, modelMatrix.v);
    }

    // ----- BLINN–PHONG MATERIAL UNIFORMS -----
    Vec3f defaultKd = {1.0f, 1.0f, 1.0f};
    float defaultNs = 32.0f;

    GLint locKd = glGetUniformLocation(currentProg, "uMaterialKd");
    GLint locNs = glGetUniformLocation(currentProg, "uMaterialShininess");

    if (locKd >= 0)
        glUniform3fv(locKd, 1, &defaultKd.x);

    if (locNs >= 0)
        glUniform1f(locNs, defaultNs);

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

void drawTerrain(
    GLuint vao,
    GLsizei vertexCount,
    GLuint texture,
    const Mat44f &projCameraWorld,
    const Mat33f &normalMatrix,
    const Mat44f &modelMatrix)
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
        modelMatrix
    );
}

void drawLandingPads(
    const std::vector<LandingPad> &pads,
    const Mat44f &projView)
{
    glUniform1i(10, 0); // coloured mode
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindVertexArray(pads[0].vao);

    GLint currentProg = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProg);

    // Uniform locations
    const GLint locProjCameraWorld = 0;
    const GLint locNormalMatrix    = 1;

    const GLint locModel           = glGetUniformLocation(currentProg, "uModel");
    const GLint locKd              = glGetUniformLocation(currentProg, "uMaterialKd");
    const GLint locNs              = glGetUniformLocation(currentProg, "uMaterialShininess");

    // Draw each landing pad
    for (const auto &pad : pads)
    {
        Mat44f modelMatrix      = pad.transform;
        Mat44f projCameraWorld  = projView * modelMatrix;
        Mat33f normalMatrix     = make_uniform_normal(modelMatrix);

        glUniformMatrix4fv(locProjCameraWorld, 1, GL_TRUE, projCameraWorld.v);
        glUniformMatrix3fv(locNormalMatrix,    1, GL_TRUE, normalMatrix.v);

        if (locModel >= 0)
            glUniformMatrix4fv(locModel, 1, GL_TRUE, modelMatrix.v);

        // Upload MTL material Kd + Ns
        if (locKd >= 0)
            glUniform3fv(locKd, 1, &pad.kd.x);

        if (locNs >= 0)
            glUniform1f(locNs, pad.shininess);

        glDrawArrays(GL_TRIANGLES, 0, pad.vertexCount);
    }
}

void drawObject(
    GLuint vao,
    GLsizei vertexCount,
    GLsizei indexCount,
    const Mat44f &projCameraWorld,
    const Mat33f &normalMatrix,
    const Mat44f &modelMatrix)
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
