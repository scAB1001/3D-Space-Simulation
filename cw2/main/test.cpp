#include "test.hpp"

#include <print>
#include <cassert>

void test_vao_creation()
{
    std::print("\n=== TESTING VAO CREATION ===\n");

    // Test 1: Indexed Cube: PASS
    {
        auto cube = make_indexed_cube({1.f, 0.f, 0.f});
        GLuint vao = create_vao(cube);
        if (vao)
        {
            std::print("✓ Indexed cube VAO created successfully: {}\n", vao);

            // Test rendering
            glBindVertexArray(vao);
            if (cube.hasIndices())
            {
                glDrawElements(GL_TRIANGLES, cube.indexCount(), GL_UNSIGNED_INT, 0);
                std::print("  Rendered with glDrawElements\n");
            }
            else
            {
                glDrawArrays(GL_TRIANGLES, 0, cube.vertexCount());
                std::print("  Rendered with glDrawArrays\n");
            }
            glBindVertexArray(0);

            glDeleteVertexArrays(1, &vao);
        }
        else
        {
            std::print("✗ Failed to create indexed cube VAO\n");
        }
    }

    // Test 2: Non-indexed Cube: PASS
    {
        auto cube = make_colored_cube({0.f, 1.f, 0.f});
        GLuint vao = create_vao(cube);
        if (vao)
        {
            std::print("✓ Non-indexed cube VAO created successfully: {}\n", vao);
            glDeleteVertexArrays(1, &vao);
        }
        else
        {
            std::print("✗ Failed to create non-indexed cube VAO\n");
        }
    }

    // Test 3: Batched Indexed Cylinder: PASS
    {
        auto cylinder = make_batched_indexed_cylinder(true, 16, {0.f, 0.f, 1.f});
        GLuint vao = create_vao(cylinder);
        if (vao)
        {
            std::print("✓ Batched indexed cylinder VAO created: {}\n", vao);
            std::print("  Vertices: {}, Indices: {}\n",
                       cylinder.vertexCount(), cylinder.indexCount());
            glDeleteVertexArrays(1, &vao);
        }
        else
        {
            std::print("✗ Failed to create cylinder VAO\n");
        }
    }

    // Test 4: Error Cases: PASS
    {
        SimpleMeshData emptyMesh;
        GLuint vao = create_vao(emptyMesh);
        if (vao == 0)
        {
            std::print("✓ Correctly rejected empty mesh\n");
        }
        else
        {
            std::print("✗ Should have rejected empty mesh\n");
            glDeleteVertexArrays(1, &vao);
        }
    }

    std::print("=== VAO CREATION TESTS COMPLETE ===\n\n");
}

// Add this function to main.cpp to test all mesh functions
void test_all_mesh_functions()
{
    std::print("\n=== TESTING MESH FUNCTIONS ===\n");

    // Test 1: Indexed Cube
    {
        auto cube = make_indexed_cube({1.f, 0.f, 0.f});
        std::print("1. Indexed Cube: {} vertices, {} indices, {} normals\n",
                   cube.vertexCount(), cube.indexCount(), cube.normals.size());
        assert(cube.vertexCount() == 8);
        assert(cube.indexCount() == 36);
        assert(cube.hasIndices());
    }

    // Test 2: Non-indexed Cube
    {
        auto cube = make_colored_cube({0.f, 1.f, 0.f});
        std::print("2. Non-indexed Cube: {} vertices, {} indices\n",
                   cube.vertexCount(), cube.indexCount());
        assert(cube.vertexCount() == 36);
        assert(!cube.hasIndices());
    }

    // Test 3: Batched Indexed Cylinder
    {
        auto cylinder = make_batched_indexed_cylinder(true, 16, {0.f, 0.f, 1.f});
        std::print("3. Batched Indexed Cylinder: {} vertices, {} indices\n",
                   cylinder.vertexCount(), cylinder.indexCount());
        assert(cylinder.hasIndices());
    }

    // Test 4: Batched Indexed Cone
    {
        auto cone = make_batched_indexed_cone(true, 16, {1.f, 1.f, 0.f});
        std::print("4. Batched Indexed Cone: {} vertices, {} indices\n",
                   cone.vertexCount(), cone.indexCount());
        assert(cone.hasIndices());
    }

    // Test 5: Concatenation
    {
        auto cube1 = make_indexed_cube({1.f, 0.f, 0.f});
        auto cube2 = make_indexed_cube({0.f, 0.f, 1.f});
        auto combined = concatenate(cube1, cube2);
        std::print("5. Concatenated cubes: {} vertices, {} indices\n",
                   combined.vertexCount(), combined.indexCount());
        assert(combined.vertexCount() == 16);
        assert(combined.indexCount() == 72);
    }

    // Test 6: VAO Creation
    {
        auto cube = make_indexed_cube({1.f, 0.f, 0.f});
        GLuint vao = create_vao(cube);
        std::print("6. Created indexed VAO: {}\n", vao);
        glDeleteVertexArrays(1, &vao);
    }

    std::print("=== ALL TESTS PASSED ===\n\n");
}