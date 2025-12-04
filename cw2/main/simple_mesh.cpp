#include "simple_mesh.hpp"

/*
SimpleMeshData concatenate( SimpleMeshData aM, SimpleMeshData const& aN )
{
    aM.positions.insert( aM.positions.end(), aN.positions.begin(), aN.positions.end() );
    aM.normals.insert( aM.normals.end(), aN.normals.begin(), aN.normals.end() );
    aM.texcoords.insert( aM.texcoords.end(), aN.texcoords.begin(), aN.texcoords.end() );
    return aM;
}

SimpleMeshData concatenate_many(std::initializer_list<std::reference_wrapper<const SimpleMeshData>> meshes)
{
    // Calculate total size
    // std::size_t totalPositions = 0;
    // std::size_t totalColors = 0;

    // for (const auto &mesh_ref : meshes)
    // {
    // 	const auto &mesh = mesh_ref.get();
    // 	totalPositions += mesh.positions.size();
    // 	totalColors += mesh.colors.size();
    // }

    // Pre-allocate memory
    SimpleMeshData result;
    // result.positions.reserve(totalPositions);
    // result.colors.reserve(totalColors);

    // // Copy data from all meshes using references to avoid copies
    // for (const auto &mesh_ref : meshes)
    // {
    // 	const auto &mesh = mesh_ref.get();
    // 	result.positions.insert(result.positions.end(), mesh.positions.begin(), mesh.positions.end());
    // 	result.colors.insert(result.colors.end(), mesh.colors.begin(), mesh.colors.end());
    // }

    return result;
}
*/

SimpleMeshData make_cube_with_normals(Vec3f color)
{
    SimpleMeshData cube;

    // Positions and normals for all 36 vertices (6 faces × 2 triangles × 3 vertices)
    const Vec3f positions[] = {
        // Front face
        {-1.0f, -1.0f,  1.0f}, { 1.0f, -1.0f,  1.0f}, { 1.0f,  1.0f,  1.0f},
        { 1.0f,  1.0f,  1.0f}, {-1.0f,  1.0f,  1.0f}, {-1.0f, -1.0f,  1.0f},
        // Back face
        {-1.0f, -1.0f, -1.0f}, {-1.0f,  1.0f, -1.0f}, { 1.0f,  1.0f, -1.0f},
        { 1.0f,  1.0f, -1.0f}, { 1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f, -1.0f},
        // Left face
        {-1.0f,  1.0f,  1.0f}, {-1.0f,  1.0f, -1.0f}, {-1.0f, -1.0f, -1.0f},
        {-1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f,  1.0f}, {-1.0f,  1.0f,  1.0f},
        // Right face
        { 1.0f,  1.0f,  1.0f}, { 1.0f, -1.0f,  1.0f}, { 1.0f, -1.0f, -1.0f},
        { 1.0f, -1.0f, -1.0f}, { 1.0f,  1.0f, -1.0f}, { 1.0f,  1.0f,  1.0f},
        // Top face
        {-1.0f,  1.0f, -1.0f}, {-1.0f,  1.0f,  1.0f}, { 1.0f,  1.0f,  1.0f},
        { 1.0f,  1.0f,  1.0f}, { 1.0f,  1.0f, -1.0f}, {-1.0f,  1.0f, -1.0f},
        // Bottom face
        {-1.0f, -1.0f, -1.0f}, { 1.0f, -1.0f, -1.0f}, { 1.0f, -1.0f,  1.0f},
        { 1.0f, -1.0f,  1.0f}, {-1.0f, -1.0f,  1.0f}, {-1.0f, -1.0f, -1.0f}
    };

    const Vec3f normals[] = {
        // Front face
        {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f},
        // Back face
        {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f},
        // Left face
        {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f},
        // Right face
        {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
        // Top face
        {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
        // Bottom face
        {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f},
        {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}
    };

    // Add to mesh
    color = {0.5f, 0.5f, 0.5f};
    for (int i = 0; i < 36; ++i)
    {
        cube.positions.push_back(positions[i]);
        cube.normals.push_back(normals[i]);
        cube.colors.push_back(color);
    }

    return cube;
}


ColoredMeshData make_colored_cube(Vec3f color)
{
    ColoredMeshData cube;

    // Positions
    const Vec3f positions[] = {
        // Front face
        {-1.0f, -1.0f,  1.0f}, { 1.0f, -1.0f,  1.0f}, { 1.0f,  1.0f,  1.0f},
        { 1.0f,  1.0f,  1.0f}, {-1.0f,  1.0f,  1.0f}, {-1.0f, -1.0f,  1.0f},
        // Back face
        {-1.0f, -1.0f, -1.0f}, {-1.0f,  1.0f, -1.0f}, { 1.0f,  1.0f, -1.0f},
        { 1.0f,  1.0f, -1.0f}, { 1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f, -1.0f},
        // Left face
        {-1.0f,  1.0f,  1.0f}, {-1.0f,  1.0f, -1.0f}, {-1.0f, -1.0f, -1.0f},
        {-1.0f, -1.0f, -1.0f}, {-1.0f, -1.0f,  1.0f}, {-1.0f,  1.0f,  1.0f},
        // Right face
        { 1.0f,  1.0f,  1.0f}, { 1.0f, -1.0f,  1.0f}, { 1.0f, -1.0f, -1.0f},
        { 1.0f, -1.0f, -1.0f}, { 1.0f,  1.0f, -1.0f}, { 1.0f,  1.0f,  1.0f},
        // Top face
        {-1.0f,  1.0f, -1.0f}, {-1.0f,  1.0f,  1.0f}, { 1.0f,  1.0f,  1.0f},
        { 1.0f,  1.0f,  1.0f}, { 1.0f,  1.0f, -1.0f}, {-1.0f,  1.0f, -1.0f},
        // Bottom face
        {-1.0f, -1.0f, -1.0f}, { 1.0f, -1.0f, -1.0f}, { 1.0f, -1.0f,  1.0f},
        { 1.0f, -1.0f,  1.0f}, {-1.0f, -1.0f,  1.0f}, {-1.0f, -1.0f, -1.0f}
    };

    // Normals
    const Vec3f normals[] = {
        // Front face
        {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f},
        // Back face
        {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f},
        {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, -1.0f},
        // Left face
        {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f},
        {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f},
        // Right face
        {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
        // Top face
        {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
        // Bottom face
        {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f},
        {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}
    };

    for (int i = 0; i < 36; ++i)
    {
        cube.positions.push_back(positions[i]);
        cube.normals.push_back(normals[i]);
        cube.colors.push_back(color);
    }

    return cube;
}

TexturedMeshData make_textured_plane()
{
    return TexturedMeshData{
        // Positions
        {
            {-1.f, -1.f, 3.f},
            {+1.f, -1.f, 3.f},
            {+1.f, -1.f, -3.f},
            {-1.f, -1.f, 3.f},
            {+1.f, -1.f, -3.f},
            {-1.f, -1.f, -3.f}},
        // Texture coordinates
        {
            {0.f, 0.f}, {1.f, 0.f}, {1.f, 3.f}, {0.f, 0.f}, {1.f, 3.f}, {0.f, 3.f}},
        // Normals (all pointing up)
        {
            {0.f, 1.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 1.f, 0.f}}};
}

GLuint create_vao(SimpleMeshData const& mesh)
{
    // TODO: Create create_vbo method
    assert(mesh.vertexCount() > 0);
    assert(mesh.hasNormals()); // We need normals for lighting

    GLuint vao, vboPositions, vboNormals, vboColors, vboTexCoords;

    // Generate buffers
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vboPositions);
    glGenBuffers(1, &vboNormals);

    if (mesh.hasColors())
    {
        glGenBuffers(1, &vboColors);
    }
    if (mesh.hasTexcoords())
    {
        glGenBuffers(1, &vboTexCoords);
    }

    // Bind VAO
    glBindVertexArray(vao);

    // Positions (location = 0)
    glBindBuffer(GL_ARRAY_BUFFER, vboPositions);
    glBufferData(GL_ARRAY_BUFFER,
                 mesh.positions.size() * sizeof(Vec3f),
                 mesh.positions.data(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    // Normals (location = 3)
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER,
                 mesh.normals.size() * sizeof(Vec3f),
                 mesh.normals.data(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(3);

    // Colors or Texcoords (both use location = 1)
    if (mesh.hasTexcoords())
    {
        // Textured mesh - use texcoords
        glBindBuffer(GL_ARRAY_BUFFER, vboTexCoords);
        glBufferData(GL_ARRAY_BUFFER,
                     mesh.texcoords.size() * sizeof(Vec2f),
                     mesh.texcoords.data(),
                     GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(1);
    }
    else if (mesh.hasColors())
    {
        // Colored mesh - use colors
        glBindBuffer(GL_ARRAY_BUFFER, vboColors);
        glBufferData(GL_ARRAY_BUFFER,
                     mesh.colors.size() * sizeof(Vec3f),
                     mesh.colors.data(),
                     GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(1);
    }
    // If neither colors nor texcoords exist, location 1 remains disabled

    // Unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return vao;
}

/*
GLuint create_vao_textured(TexturedMeshData const& mesh)
{
    GLuint vao, vboPositions, vboTexCoords, vboNormals;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vboPositions);
    glGenBuffers(1, &vboTexCoords);
    glGenBuffers(1, &vboNormals);

    glBindVertexArray(vao);

    // Positions (location = 0)
    glBindBuffer(GL_ARRAY_BUFFER, vboPositions);
    glBufferData(GL_ARRAY_BUFFER,
                 mesh.positions.size() * sizeof(Vec3f),
                 mesh.positions.data(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    // Texture coordinates (location = 1)
    glBindBuffer(GL_ARRAY_BUFFER, vboTexCoords);
    glBufferData(GL_ARRAY_BUFFER,
                 mesh.texcoords.size() * sizeof(Vec2f),
                 mesh.texcoords.data(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);

    // Normals (location = 2)
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER,
                 mesh.normals.size() * sizeof(Vec3f),
                 mesh.normals.data(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return vao;
}

GLuint create_vao_colored(ColoredMeshData const& mesh)
{
    GLuint vao, vboPositions, vboColors, vboNormals;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vboPositions);
    glGenBuffers(1, &vboColors);
    glGenBuffers(1, &vboNormals);

    glBindVertexArray(vao);

    // Positions (location = 0)
    glBindBuffer(GL_ARRAY_BUFFER, vboPositions);
    glBufferData(GL_ARRAY_BUFFER,
                 mesh.positions.size() * sizeof(Vec3f),
                 mesh.positions.data(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    // Colors (location = 1)
    glBindBuffer(GL_ARRAY_BUFFER, vboColors);
    glBufferData(GL_ARRAY_BUFFER,
                 mesh.colors.size() * sizeof(Vec3f),
                 mesh.colors.data(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);

    // Normals (location = 2)
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER,
                 mesh.normals.size() * sizeof(Vec3f),
                 mesh.normals.data(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return vao;
}

GLuint create_vao_color(SimpleMeshDataColor const &aMeshData)
{
	// Create VAO and VBOs
	GLuint vao, vboPositions, vboColors, vboNormals;

	// Generate buffers
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vboPositions);
	glGenBuffers(1, &vboColors);
	glGenBuffers(1, &vboNormals);

	// Bind VAO
	glBindVertexArray(vao);

	// Positions VBO
	glBindBuffer(GL_ARRAY_BUFFER, vboPositions);
	glBufferData(GL_ARRAY_BUFFER,
				 aMeshData.positions.size() * sizeof(Vec3f),
				 aMeshData.positions.data(),
				 GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glEnableVertexAttribArray(0);

	// Colors VBO
	glBindBuffer(GL_ARRAY_BUFFER, vboColors);
	glBufferData(GL_ARRAY_BUFFER,
				 aMeshData.colors.size() * sizeof(Vec3f),
				 aMeshData.colors.data(),
				 GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glEnableVertexAttribArray(1);

	// TODO: Include normals VBO
	glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
	glBufferData(GL_ARRAY_BUFFER,
				 aMeshData.normals.size() * sizeof(Vec3f),
				 aMeshData.normals.data(),
				 GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glEnableVertexAttribArray(2);

	// Unbind
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return vao;
}

GLuint create_vao(SimpleMeshData const &aMeshData)
{
	// Create VAO and VBOs
	GLuint vao, vboPositions, vboNormals, vboTexCoords;

	// Generate buffers
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vboPositions);
	glGenBuffers(1, &vboNormals);
	glGenBuffers(1, &vboTexCoords);

	// Bind VAO
	glBindVertexArray(vao);

	// Positions VBO
	glBindBuffer(GL_ARRAY_BUFFER, vboPositions);
	glBufferData(GL_ARRAY_BUFFER,
				 aMeshData.positions.size() * sizeof(Vec3f),
				 aMeshData.positions.data(),
				 GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glEnableVertexAttribArray(0);

	// Texcoords VBO
	glBindBuffer(GL_ARRAY_BUFFER, vboTexCoords);
	glBufferData(GL_ARRAY_BUFFER,
		aMeshData.texcoords.size() * sizeof(Vec3f),
		aMeshData.texcoords.data(),
		GL_STATIC_DRAW);

	// 2D Vectors for texcoords
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glEnableVertexAttribArray(1);

	// Normals VBO
	glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
	glBufferData(GL_ARRAY_BUFFER,
		aMeshData.normals.size() * sizeof(Vec3f),
		aMeshData.normals.data(),
		GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glEnableVertexAttribArray(2);

	// Unbind
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return vao;
}

GLuint create_vao_from_meshes(std::initializer_list<std::reference_wrapper<const SimpleMeshData>> meshes)
{
	GLuint vao, vboPositions, vboColors, vboNormals;

	// std::size_t totalVertices = 0;
	// std::vector<std::size_t> meshSizes;
	// meshSizes.reserve(meshes.size());

	// // Calculate total size
	// for (const auto &mesh_ref : meshes)
	// {
	// 	const auto &mesh = mesh_ref.get();
	// 	std::size_t size = mesh.positions.size();

	// 	// Ensure all arrays have same size
	// 	assert(mesh.colors.size() == size && mesh.normals.size() == size);

	// 	meshSizes.push_back(size);
	// 	totalVertices += size;
	// }

	// // Generate buffers
	// glGenVertexArrays(1, &vao);
	// glGenBuffers(1, &vboPositions);
	// glGenBuffers(1, &vboColors);
	// glGenBuffers(1, &vboNormals);

	// glBindVertexArray(vao);

	// // Pre-compute total byte size
	// const std::size_t totalBytes = totalVertices * sizeof(Vec3f);

	// // POSITIONS BUFFER
	// glBindBuffer(GL_ARRAY_BUFFER, vboPositions);
	// glBufferData(GL_ARRAY_BUFFER, totalBytes, nullptr, GL_STATIC_DRAW);

	// // COLORS BUFFER
	// glBindBuffer(GL_ARRAY_BUFFER, vboColors);
	// glBufferData(GL_ARRAY_BUFFER, totalBytes, nullptr, GL_STATIC_DRAW);

	// // NORMALS BUFFER
	// glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
	// glBufferData(GL_ARRAY_BUFFER, totalBytes, nullptr, GL_STATIC_DRAW);

	// // Upload data from all meshes
	// std::size_t vertexOffset = 0;
	// for (const auto &mesh_ref : meshes)
	// {
	// 	const auto &mesh = mesh_ref.get();
	// 	std::size_t meshVertexCount = mesh.positions.size();

	// 	if (meshVertexCount > 0)
	// 	{
	// 		const std::size_t byteOffset = vertexOffset * sizeof(Vec3f);
	// 		const std::size_t byteSize = meshVertexCount * sizeof(Vec3f);

	// 		// Upload positions
	// 		glBindBuffer(GL_ARRAY_BUFFER, vboPositions);
	// 		glBufferSubData(GL_ARRAY_BUFFER, byteOffset, byteSize, mesh.positions.data());

	// 		// Upload colors
	// 		glBindBuffer(GL_ARRAY_BUFFER, vboColors);
	// 		glBufferSubData(GL_ARRAY_BUFFER, byteOffset, byteSize, mesh.colors.data());

	// 		// Upload normals
	// 		glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
	// 		glBufferSubData(GL_ARRAY_BUFFER, byteOffset, byteSize, mesh.normals.data());
	// 	}

	// 	vertexOffset += meshVertexCount;
	// }

	// // SETUP VERTEX ATTRIBUTES
	// // Position attribute (location = 0)
	// glBindBuffer(GL_ARRAY_BUFFER, vboPositions);
	// glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	// glEnableVertexAttribArray(0);

	// // Color attribute (location = 1)
	// glBindBuffer(GL_ARRAY_BUFFER, vboColors);
	// glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	// glEnableVertexAttribArray(1);

	// // Normal attribute (location = 2)
	// glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
	// glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	// glEnableVertexAttribArray(2);

	// // Unbind
	// glBindVertexArray(0);
	// glBindBuffer(GL_ARRAY_BUFFER, 0);

	return vao;
}
*/