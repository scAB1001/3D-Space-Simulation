#include "cube.hpp"

SimpleMeshData make_indexed_cube(Vec3f color)
{
    SimpleMeshData cube;

    // 8 unique vertices
    cube.positions = {
        // Front face
        {-1.0f, -1.0f, 1.0f},  // 0
        {1.0f, -1.0f, 1.0f},   // 1
        {1.0f, 1.0f, 1.0f},    // 2
        {-1.0f, 1.0f, 1.0f},   // 3
        // Back face
        {-1.0f, -1.0f, -1.0f}, // 4
        {1.0f, -1.0f, -1.0f},  // 5
        {1.0f, 1.0f, -1.0f},   // 6
        {-1.0f, 1.0f, -1.0f}   // 7
    };

    // 6 faces * 2 triangles * 3 indices = 36 indices
    cube.indices = {
        // Front
        0, 1, 2, 2, 3, 0,
        // Back
        5, 4, 7, 7, 6, 5,
        // Left
        4, 0, 3, 3, 7, 4,
        // Right
        1, 5, 6, 6, 2, 1,
        // Top
        3, 2, 6, 6, 7, 3,
        // Bottom
        4, 5, 1, 1, 0, 4};

    // Colors and normals
    cube.colors.resize(8, color);
    cube.calculate_normals();

    return cube;
}