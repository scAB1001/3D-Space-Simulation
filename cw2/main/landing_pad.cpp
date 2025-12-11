#include "landing_pad.hpp"

// Static member initialization
SimpleMeshData LandingPad::meshData;
GLuint LandingPad::sharedVao = 0;
bool LandingPad::initialized = false;
std::vector<Material> LandingPad::materials;

void LandingPad::initialize()
{
    if (initialized)
        return;

    // Load mesh once
    meshData = load_wavefront_obj_with_mtl(
        "assets/cw2/landingpad.obj",
        materials);

    // Create VAO once
    sharedVao = create_vao(meshData);
    initialized = true;
}

void LandingPad::cleanup()
{
    if (sharedVao)
    {
        glDeleteVertexArrays(1, &sharedVao);
        sharedVao = 0;
    }
    initialized = false;
}

LandingPad::LandingPad(Vec3f position, float scale) : position(position)
{
    if (!initialized)
        initialize();

    this->vao = sharedVao;
    this->vertexCount = meshData.vertexCount();
    this->transform = make_translation(position) * make_scaling(scale, scale, scale);

    // ----- Assign material -----
    if (!materials.empty())
    {
        kd = materials[0].diffuse;      // Kd
        shininess = materials[0].shininess; // Ns
    }
    else
    {
        kd = Vec3f{1.0f, 1.0f, 1.0f};
        shininess = 32.0f;
    }
}