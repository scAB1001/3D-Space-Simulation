#include "simple_mesh.hpp"
#include <print>

/* References for code inspiration:
 * - https://learnopengl.com/Model-Loading/Mesh
 * - https://www.gamedev.net/articles/programming/graphics/opengl-batch-rendering-r3900/
 * - https://github.com/robmaier/menderer
 * - https://www.opengl-tutorial.org/intermediate-tutorials/tutorial-9-vbo-indexing/
 * - https://learnopengl.com/Getting-started/Hello-Triangle
 */
void apply_transform_to_mesh(SimpleMeshData &mesh, Mat44f const &M)
{
    for (std::size_t i = 0; i < mesh.positions.size(); ++i)
    {
        Vec3f p = mesh.positions[i];
        Vec4f p4 = M * Vec4f{p.x, p.y, p.z, 1.f};
        mesh.positions[i] = Vec3f{p4.x, p4.y, p4.z};
    }
}

SimpleMeshData concatenate(SimpleMeshData aM, SimpleMeshData const &aN)
{
    // ---------- Check material type compatibility ----------
    if (aM.materialType != aN.materialType)
    {
        std::print(stderr, "ERROR [concatenate]: Cannot mix material types {} and {}\n",
                   aM.materialType, aN.materialType);
        return aM; // Return original mesh unchanged
    }

    // Check attribute consistency
    if (aM.hasTexcoords() != aN.hasTexcoords())
    {
        std::print(stderr, "ERROR [concatenate]: Texcoord presence mismatch\n");
        return aM;
    }

    if (aM.hasColors() != aN.hasColors())
    {
        std::print(stderr, "ERROR [concatenate]: Color presence mismatch\n");
        return aM;
    }

    // ---------- Handle indexed meshes ----------
    if (aM.hasIndices() || aN.hasIndices())
    {
        // For indexed meshes, we need to adjust indices
        std::size_t offset = aM.positions.size();

        // Concatenate positions, normals (ALWAYS)
        aM.positions.insert(aM.positions.end(), aN.positions.begin(), aN.positions.end());
        aM.normals.insert(aM.normals.end(), aN.normals.begin(), aN.normals.end());

        // Concatenate colors or texcoords based on material type
        if (aM.materialType == 0 && aM.hasColors() && aN.hasColors())
        {
            // Colored mode: concatenate colors
            aM.colors.insert(aM.colors.end(), aN.colors.begin(), aN.colors.end());
        }
        else if (aM.materialType == 1 && aM.hasTexcoords() && aN.hasTexcoords())
        {
            // Textured mode: concatenate texcoords
            aM.texcoords.insert(aM.texcoords.end(), aN.texcoords.begin(), aN.texcoords.end());
        }

        // Concatenate indices with offset
        if (aN.hasIndices())
        {
            if (!aM.hasIndices())
            {
                // If aM wasn't indexed but aN is, create indices for aM first
                for (std::size_t i = 0; i < offset; ++i) // FIXED: Use offset, not aM.positions.size() - offset
                    aM.indices.push_back(static_cast<unsigned int>(i));
            }

            // Add aN's indices with offset
            for (auto index : aN.indices)
                aM.indices.push_back(static_cast<unsigned int>(index + offset));
        }
        else if (aM.hasIndices())
        {
            // aM is indexed but aN isn't - create indices for aN
            for (std::size_t i = 0; i < aN.positions.size(); ++i)
                aM.indices.push_back(static_cast<unsigned int>(offset + i));
        }
    }
    else
    {
        // Concatenate positions and normals
        aM.positions.insert(aM.positions.end(), aN.positions.begin(), aN.positions.end());
        aM.normals.insert(aM.normals.end(), aN.normals.begin(), aN.normals.end());

        // Concatenate colors or texcoords based on material type
        if (aM.materialType == 0 && aM.hasColors() && aN.hasColors())
        {
            aM.colors.insert(aM.colors.end(), aN.colors.begin(), aN.colors.end());
        }
        else if (aM.materialType == 1 && aM.hasTexcoords() && aN.hasTexcoords())
        {
            aM.texcoords.insert(aM.texcoords.end(), aN.texcoords.begin(), aN.texcoords.end());
        }
    }

    return aM;
}

SimpleMeshData concatenate_many(std::initializer_list<std::reference_wrapper<const SimpleMeshData>> meshes)
{
    if (meshes.size() == 0)
    {
        std::print(stderr, "ERROR [concatenate_many]: Empty mesh list\n");
        return SimpleMeshData();
    }

    // ---------- VALIDATION ----------
    const SimpleMeshData &firstMesh = meshes.begin()->get();
    int expectedMaterialType = firstMesh.materialType;
    bool expectColors = firstMesh.hasColors();
    bool expectTexcoords = firstMesh.hasTexcoords();

    // Verify all meshes have consistent attributes
    for (const auto &mesh_ref : meshes)
    {
        const auto &mesh = mesh_ref.get();

        if (mesh.materialType != expectedMaterialType)
        {
            std::print(stderr, "ERROR [concatenate_many]: Mixed material types ({} != {})\n",
                       mesh.materialType, expectedMaterialType);
            return SimpleMeshData();
        }

        if (mesh.hasColors() != expectColors)
        {
            std::print(stderr, "ERROR [concatenate_many]: Inconsistent color presence\n");
            return SimpleMeshData();
        }

        if (mesh.hasTexcoords() != expectTexcoords)
        {
            std::print(stderr, "ERROR [concatenate_many]: Inconsistent texcoord presence\n");
            return SimpleMeshData();
        }

        if (!mesh.hasNormals())
        {
            std::print(stderr, "ERROR [concatenate_many]: Mesh missing normals\n");
            return SimpleMeshData();
        }
    }

    // ---------- CALCULATE SIZES ----------
    std::size_t totalPositions = 0;
    std::size_t totalColors = 0;
    std::size_t totalNormals = 0;
    std::size_t totalTexcoords = 0;
    std::size_t totalIndices = 0;

    bool hasIndices = false;

    for (const auto &mesh_ref : meshes)
    {
        const auto &mesh = mesh_ref.get();
        totalPositions += mesh.positions.size();
        totalNormals += mesh.normals.size();

        if (expectColors)
            totalColors += mesh.colors.size();
        if (expectTexcoords)
            totalTexcoords += mesh.texcoords.size();

        totalIndices += mesh.indices.size();

        if (mesh.hasIndices())
            hasIndices = true;
    }

    // ---------- PRE-ALLOCATE ----------
    SimpleMeshData result;
    result.materialType = expectedMaterialType;
    result.positions.reserve(totalPositions);
    result.normals.reserve(totalNormals);

    if (expectColors)
        result.colors.reserve(totalColors);
    if (expectTexcoords)
        result.texcoords.reserve(totalTexcoords);

    if (hasIndices)
        result.indices.reserve(totalIndices);

    // ---------- CONCATENATE ----------
    std::size_t vertexOffset = 0;

    for (const auto &mesh_ref : meshes)
    {
        const auto &mesh = mesh_ref.get();

        // Concatenate vertex data
        result.positions.insert(result.positions.end(),
                                mesh.positions.begin(), mesh.positions.end());
        result.normals.insert(result.normals.end(),
                              mesh.normals.begin(), mesh.normals.end());

        if (expectColors && mesh.hasColors())
        {
            result.colors.insert(result.colors.end(),
                                 mesh.colors.begin(), mesh.colors.end());
        }

        if (expectTexcoords && mesh.hasTexcoords())
        {
            result.texcoords.insert(result.texcoords.end(),
                                    mesh.texcoords.begin(), mesh.texcoords.end());
        }

        // Concatenate indices with offset
        if (mesh.hasIndices())
        {
            for (auto index : mesh.indices)
                result.indices.push_back(static_cast<unsigned int>(index + vertexOffset));
        }
        else if (hasIndices)
        {
            // If result will be indexed but this mesh isn't, create indices for it
            for (std::size_t i = 0; i < mesh.positions.size(); ++i)
                result.indices.push_back(static_cast<unsigned int>(vertexOffset + i));
        }

        vertexOffset += mesh.positions.size();
    }

    // ---------- POST-VALIDATION ----------
    // Ensure all arrays have correct sizes
    if (result.positions.size() != result.normals.size())
    {
        std::print(stderr, "ERROR [concatenate_many]: Position/Normal count mismatch after concatenation\n");
        return SimpleMeshData();
    }

    if (expectColors && result.colors.size() != result.positions.size())
    {
        std::print(stderr, "ERROR [concatenate_many]: Color count mismatch after concatenation\n");
        return SimpleMeshData();
    }

    if (expectTexcoords && result.texcoords.size() != result.positions.size())
    {
        std::print(stderr, "ERROR [concatenate_many]: Texcoord count mismatch after concatenation\n");
        return SimpleMeshData();
    }

    return result;
}

// Buffer and Attribute Object creation functions
GLuint create_bo(GLenum target, const void *data, std::size_t dataSize, GLint attribIndex, GLint attribSize, GLenum usage)
{
    // ------- Parameter Validation -------
    // Check for valid buffer target
    if (target != GL_ARRAY_BUFFER && target != GL_ELEMENT_ARRAY_BUFFER)
    {
        std::print(stderr, "ERROR [create_bo]: Invalid buffer target {}\n", target);
        return 0;
    }

    // Check for zero data size (allowed for dynamic buffers)
    if (dataSize == 0)
    {
        std::print(stderr, "WARNING [create_bo]: Zero data size (may be intentional for dynamic buffer)\n");
    }

    // Check for null data with non-zero size
    if (!data && dataSize > 0)
    {
        std::print(stderr, "ERROR [create_bo]: Null data pointer with non-zero size {}\n", dataSize);
        return 0;
    }

    // Validate vertex attribute parameters for array buffers
    if (target == GL_ARRAY_BUFFER)
    {
        // Check if attribute setup requested
        if (attribIndex != -1 || attribSize != -1)
        {
            // Both parameters must be provided
            if (attribIndex == -1 || attribSize == -1)
            {
                std::print(stderr, "ERROR [create_bo]: Must provide both attribIndex and attribSize for ARRAY_BUFFER\n");
                return 0;
            }

            // Validate attribute index range
            GLint maxAttribs;
            glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttribs);
            if (attribIndex < 0 || attribIndex >= maxAttribs)
            {
                std::print(stderr, "ERROR [create_bo]: Invalid vertex attribute index {} (max: {})\n", attribIndex, maxAttribs - 1);
                return 0;
            }

            // Validate attribute size
            if (attribSize < 1 || attribSize > 4)
            {
                std::print(stderr, "ERROR [create_bo]: Invalid vertex attribute size {} (must be 1-4)\n", attribSize);
                return 0;
            }
        }
    }
    else // GL_ELEMENT_ARRAY_BUFFER
    {
        // EBO should not have vertex attribute parameters
        if (attribIndex != -1 || attribSize != -1)
        {
            std::print(stderr, "WARNING [create_bo]: attribIndex/attribSize ignored for ELEMENT_ARRAY_BUFFER\n");
        }
    }

    // ------- Buffer Creation -------
    GLuint bo = 0;
    glGenBuffers(1, &bo);

    if (bo == 0)
    {
        std::print(stderr, "ERROR [create_bo]: Failed to generate buffer object\n");
        return 0;
    }

    // Bind and upload data
    glBindBuffer(target, bo);
    glBufferData(target, dataSize, data, usage);

    // ------- Vertex Attribute Setup (only for ARRAY_BUFFER) -------
    if (target == GL_ARRAY_BUFFER && attribIndex != -1)
    {
        // Note: We use location 3 for normals to match unified shader
        // unified.vert: layout(location = 3) in vec3 iNormal;

        glVertexAttribPointer(attribIndex, attribSize, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(attribIndex);
    }

    // ------- Error Checking -------
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::print(stderr, "ERROR [create_bo]: OpenGL error {}\n", error);
        glDeleteBuffers(1, &bo);
        return 0;
    }

    return bo;
}

GLuint create_vao(SimpleMeshData const &mesh)
{
    // ------- Mesh validation -------
    if (mesh.vertexCount() == 0)
    {
        std::print(stderr, "ERROR: create_vao called with empty mesh\n");
        return 0;
    }

    if (!mesh.hasNormals())
    {
        std::print(stderr, "ERROR: Mesh must have normals for lighting\n");
        return 0;
    }

    // ------- Array Size validation -------
    // Validate array sizes match
    if (mesh.hasColors() && mesh.colors.size() != mesh.positions.size())
    {
        std::print(stderr, "ERROR: Color array size mismatch: {} != {}\n",
                   mesh.colors.size(), mesh.positions.size());
        return 0;
    }

    if (mesh.hasTexcoords() && mesh.texcoords.size() != mesh.positions.size())
    {
        std::print(stderr, "ERROR: Texcoord array size mismatch: {} != {}\n",
                   mesh.texcoords.size(), mesh.positions.size());
        return 0;
    }

    if (mesh.normals.size() != mesh.positions.size())
    {
        std::print(stderr, "ERROR: Normal array size mismatch: {} != {}\n",
                   mesh.normals.size(), mesh.positions.size());
        return 0;
    }

    // ------- Material validation -------
    // Validate material type requirements
    if (mesh.materialType == 0 && !mesh.hasColors())
    {
        std::print(stderr, "ERROR: Colored mesh (materialType=0) must have colors\n");
        return 0;
    }

    if (mesh.materialType == 1 && !mesh.hasTexcoords())
    {
        std::print(stderr, "WARNING: Textured mesh (materialType=1) has no texcoords\n");
        // Not fatal, but unexpected
    }

    // ------- VAO Creation -------
    GLuint vao;
    glGenVertexArrays(1, &vao);
    if (vao == 0) {
        std::print(stderr, "ERROR: Failed to generate vertex array object\n");
        return 0;
    }

    glBindVertexArray(vao);

    // Store buffer IDs for cleanup if needed
    std::vector<GLuint> buffers;

    try
    {
        // Create position VBO
        GLuint posVbo = create_bo(GL_ARRAY_BUFFER,
                                  mesh.positions.data(),
                                  mesh.positions.size() * sizeof(Vec3f),
                                  0, 3); // location=0, size=3

        if (posVbo == 0)
            throw std::runtime_error("Failed to create position VBO");
        buffers.push_back(posVbo);

        // Create normal VBO
        GLuint normVbo = create_bo(GL_ARRAY_BUFFER,
                                   mesh.normals.data(),
                                   mesh.normals.size() * sizeof(Vec3f),
                                   3, 3); // location=3, size=3

        if (normVbo == 0)
            throw std::runtime_error("Failed to create normal VBO");
        buffers.push_back(normVbo);

        // Create color/texcoord VBO
        if (mesh.materialType == 1 && mesh.hasTexcoords())
        {
            GLuint texVbo = create_bo(GL_ARRAY_BUFFER,
                                      mesh.texcoords.data(),
                                      mesh.texcoords.size() * sizeof(Vec2f),
                                      1, 2); // location=1, size=2

            if (texVbo == 0)
                throw std::runtime_error("Failed to create texcoord VBO");
            buffers.push_back(texVbo);
        }
        else if (mesh.materialType == 0)
        {
            // Colored mesh - colors are REQUIRED for colored mode
            if (!mesh.hasColors())
            {
                throw std::runtime_error("Colored mesh has no colors");
            }

            GLuint colVbo = create_bo(GL_ARRAY_BUFFER,
                                      mesh.colors.data(),
                                      mesh.colors.size() * sizeof(Vec3f),
                                      1, 3);
            if (colVbo == 0)
                throw std::runtime_error("Failed to create color VBO");
            buffers.push_back(colVbo);
        }
        else
        {
            // Invalid material type or missing data
            throw std::runtime_error("Invalid material configuration");
        }

        // Create element buffer if indexed
        if (mesh.hasIndices())
        {
            // Validate indices
            for (size_t i = 0; i < mesh.indices.size(); ++i)
            {
                if (mesh.indices[i] >= mesh.positions.size())
                {
                    std::print(stderr, "ERROR: Index out of bounds: {} >= {}\n",
                                mesh.indices[i], mesh.positions.size());
                    throw std::runtime_error("Invalid index in mesh");
                }
            }

            // Check index count is multiple of 3 (triangles)
            if (mesh.indices.size() % 3 != 0)
            {
                std::print(stderr, "ERROR: Index count {} is not multiple of 3\n",
                           mesh.indices.size());
                throw std::runtime_error("Invalid triangle count");
            }

            // No vertex attrib parameters for EBO
            GLuint ebo = create_bo(GL_ELEMENT_ARRAY_BUFFER,
                                   mesh.indices.data(),
                                   mesh.indices.size() * sizeof(unsigned int));

            if (ebo == 0)
                throw std::runtime_error("Failed to create element buffer");
            buffers.push_back(ebo);
        }

        // Unbind VAO (saves all state including bound EBO)
        glBindVertexArray(0);
        return vao;
    }
    catch (const std::exception &e)
    {
        // Cleanup on error
        std::print(stderr, "ERROR creating VAO: {}\n", e.what());

        if (vao != 0)
            glDeleteVertexArrays(1, &vao);

        if (!buffers.empty())
            glDeleteBuffers(buffers.size(), buffers.data());

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        return 0;
    }

    return vao;
}

GLuint create_vao_from_meshes(std::initializer_list<std::reference_wrapper<const SimpleMeshData>> meshes)
{
    // Use concatenate_many to merge meshes
    SimpleMeshData combinedMesh = concatenate_many(meshes);

    if (combinedMesh.positions.empty())
    {
        std::print(stderr, "ERROR: concatenate_many returned empty mesh\n");
        return 0;
    }
    return create_vao(combinedMesh);
}