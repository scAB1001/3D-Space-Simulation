#ifndef SIMPLE_MESH_HPP
#define SIMPLE_MESH_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <string>
#include <cstdlib>
#include <numbers>

#include "../vmlib/vec2.hpp"
#include "../vmlib/vec3.hpp"
#include "../vmlib/vec4.hpp"
#include "../vmlib/mat44.hpp"

struct SimpleMeshData
{
	std::vector<Vec3f> positions;
	std::vector<Vec3f> colors;		   // For colored objects (materialType=0)
	std::vector<Vec2f> texcoords;	   // For textured objects (materialType=1)
	std::vector<Vec3f> normals;		   // For lighting calculations
	std::vector<unsigned int> indices; // For indexed rendering (optional)

	int materialType = 0; // 0 = colored, 1 = textured

	// Helper functions for validation
	bool hasColors() const { return !colors.empty(); }
	bool hasTexcoords() const { return !texcoords.empty(); }
	bool hasNormals() const { return !normals.empty(); }
	bool hasIndices() const { return !indices.empty(); }

	std::size_t vertexCount() const { return positions.size(); }
	std::size_t indexCount() const { return indices.size(); }

	void clear()
	{
		positions.clear();
		colors.clear();
		texcoords.clear();
		normals.clear();
		indices.clear();
	}

	void calculate_normals()
	{
		if (!hasIndices() || positions.empty())
			return;

		normals.resize(positions.size(), Vec3f{0.f, 0.f, 0.f});

		// For each triangle
		for (size_t i = 0; i < indices.size(); i += 3)
		{
			unsigned int i0 = indices[i];
			unsigned int i1 = indices[i + 1];
			unsigned int i2 = indices[i + 2];

			Vec3f v0 = positions[i0];
			Vec3f v1 = positions[i1];
			Vec3f v2 = positions[i2];

			Vec3f edge1 = v1 - v0;
			Vec3f edge2 = v2 - v0;
			Vec3f normal = normalize(cross(edge1, edge2));

			// Check for degenerate triangle
			if (length(normal) > 0.001f)
			{
				// Add to each vertex (smooth normals)
				normals[i0] += normal;
				normals[i1] += normal;
				normals[i2] += normal;
			}
		}

		// Normalize all normals
		for (auto &n : normals)
		{
			if (length(n) > 0.f)
				n = normalize(n);
			else
				n = Vec3f{0.f, 1.f, 0.f}; // Default up
		}
	}
};

void apply_transform_to_mesh(SimpleMeshData &mesh, Mat44f const &M);
SimpleMeshData concatenate(SimpleMeshData aM, SimpleMeshData const &aN);
SimpleMeshData concatenate_many(std::initializer_list<std::reference_wrapper<const SimpleMeshData>> meshes);

GLuint create_bo(GLenum target, const void *data, std::size_t dataSize,
				 GLint attribIndex = -1, GLint attribSize = -1,
				 GLenum usage = GL_STATIC_DRAW);

GLuint create_vao(SimpleMeshData const &mesh);

GLuint create_vao_from_meshes(std::initializer_list<std::reference_wrapper<const SimpleMeshData>> meshes);

#endif // SIMPLE_MESH_HPP
