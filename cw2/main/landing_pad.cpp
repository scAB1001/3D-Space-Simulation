#include "landing_pad.hpp"
#include <print>

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

    std::println("Loaded landing pad with {} vertices", meshData.positions.size());
    std::println("Landing pad has {} materials", materials.size());

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

LandingPad::LandingPad(Vec3f position, float scale)
    : position(position)
{
    if (!initialized)
        initialize();

    this->vao = sharedVao;
    this->vertexCount = meshData.vertexCount();
    this->transform = make_translation(position) * make_scaling(scale, scale, scale);
}