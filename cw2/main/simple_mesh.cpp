#include "simple_mesh.hpp"

SimpleMeshData create_colored_arrow(const SimpleMeshData &baseArrow, Vec3f color, Mat44f transform)
{
	// Take baseArrow by const reference to avoid copy
	SimpleMeshData coloredArrow;
	coloredArrow.positions.reserve(baseArrow.positions.size());
	coloredArrow.colors.reserve(baseArrow.colors.size());

	// Transform positions
	for (const auto &pos : baseArrow.positions)
	{
		Vec4f transformed = transform * Vec4f{pos.x, pos.y, pos.z, 1.f};
		coloredArrow.positions.emplace_back(Vec3f{transformed.x, transformed.y, transformed.z});
	}

	// Set all shapes to same color
	// coloredArrow.colors.assign(baseArrow.positions.size(), color);

	// Preserve original colors from baseArrow instead of overwriting
	coloredArrow.colors = baseArrow.colors;

	// Now color only the cylinder vertices (first half)
	size_t cylinderVertexCount = baseArrow.positions.size() / 2; // Assuming cylinder is first half
	for (size_t i = 0; i < cylinderVertexCount; ++i)
	{
		coloredArrow.colors[i] = color;
	}

	return coloredArrow;
}

SimpleMeshData concatenate(SimpleMeshData aM, SimpleMeshData const &aN)
{
	aM.positions.insert(aM.positions.end(), aN.positions.begin(), aN.positions.end());
	aM.colors.insert(aM.colors.end(), aN.colors.begin(), aN.colors.end());
	return aM;
}

SimpleMeshData concatenate_many(std::initializer_list<std::reference_wrapper<const SimpleMeshData>> meshes)
{
	// Calculate total size
	std::size_t totalPositions = 0;
	std::size_t totalColors = 0;

	for (const auto &mesh_ref : meshes)
	{
		const auto &mesh = mesh_ref.get();
		totalPositions += mesh.positions.size();
		totalColors += mesh.colors.size();
	}

	// Pre-allocate memory
	SimpleMeshData result;
	result.positions.reserve(totalPositions);
	result.colors.reserve(totalColors);

	// Copy data from all meshes using references to avoid copies
	for (const auto &mesh_ref : meshes)
	{
		const auto &mesh = mesh_ref.get();
		result.positions.insert(result.positions.end(), mesh.positions.begin(), mesh.positions.end());
		result.colors.insert(result.colors.end(), mesh.colors.begin(), mesh.colors.end());
	}

	return result;
}

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

GLuint create_vao(SimpleMeshData const &aMeshData)
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

GLuint create_vao_from_meshes(std::initializer_list<std::reference_wrapper<const SimpleMeshData>> meshes)
{
	GLuint vao, vboPositions, vboColors;

	std::size_t totalVertices = 0;
	std::vector<std::size_t> meshSizes; // Store sizes contiguously
	meshSizes.reserve(meshes.size());	// Pre-allocate to avoid reallocations

	// Calculate total size and individual cache sizes
	for (const auto &mesh_ref : meshes)
	{
		std::size_t size = mesh_ref.get().positions.size();
		meshSizes.push_back(size);
		totalVertices += size;
	}

	// Generate buffers
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vboPositions);
	glGenBuffers(1, &vboColors);

	glBindVertexArray(vao);

	// Pre-compute total byte size for buffer allocation
	const std::size_t totalBytes = totalVertices * sizeof(Vec3f);

	// Positions VBO
	glBindBuffer(GL_ARRAY_BUFFER, vboPositions);
	glBufferData(GL_ARRAY_BUFFER, totalBytes, nullptr, GL_STATIC_DRAW);

	// Colors VBO
	glBindBuffer(GL_ARRAY_BUFFER, vboColors);
	glBufferData(GL_ARRAY_BUFFER, totalBytes, nullptr, GL_STATIC_DRAW);

	// Upload data using glBufferSubData
	std::size_t vertexOffset = 0;
	for (const auto &mesh_ref : meshes)
	{
		// Get ref to current mesh
		const auto &mesh = mesh_ref.get();

		std::size_t meshVertexCount = mesh.positions.size();

		// Check if there are vertices to upload
		if (meshVertexCount > 0)
		{
			// Pre-compute sizes
			const std::size_t byteOffset = vertexOffset * sizeof(Vec3f);
			const std::size_t byteSize = meshVertexCount * sizeof(Vec3f);

			// Upload positions
			glBindBuffer(GL_ARRAY_BUFFER, vboPositions);
			glBufferSubData(GL_ARRAY_BUFFER,
							byteOffset,
							byteSize,
							mesh.positions.data());

			// Upload colors
			glBindBuffer(GL_ARRAY_BUFFER, vboColors);
			glBufferSubData(GL_ARRAY_BUFFER,
							byteOffset,
							byteSize,
							mesh.colors.data());
		}

		vertexOffset += meshVertexCount;
	}

	// Set up vertex attributes
	glBindBuffer(GL_ARRAY_BUFFER, vboPositions);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vboColors);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glEnableVertexAttribArray(1);

	// Unbind
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return vao;
}