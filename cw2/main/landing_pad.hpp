#ifndef LANDING_PAD_HPP
#define LANDING_PAD_HPP

#include "loadobj.hpp"
#include "simple_mesh.hpp"
#include "../vmlib/vec3.hpp"
#include "../vmlib/mat44.hpp"
#include <vector>
#include <glad/glad.h>

struct LandingPad
{
    Vec3f position;
    Mat44f transform;
    GLuint vao;
    std::size_t vertexCount;

    // Static data shared by all landing pads
    static SimpleMeshData meshData;
    static std::vector<Material> materials;
    static GLuint sharedVao;
    static bool initialized;

    // Initialize shared resources
    static void initialize();

    // Cleanup shared resources
    static void cleanup();

    // Create landing pad instance
    LandingPad(Vec3f position, float scale = 2.0f);
};

#endif // LANDING_PAD_HPP