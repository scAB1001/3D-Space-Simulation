#ifndef SIMPLE_MESH_HPP_C6B749D6_C83B_434C_9E58_F05FC27FEFC9
#define SIMPLE_MESH_HPP_C6B749D6_C83B_434C_9E58_F05FC27FEFC9

#include <glad/glad.h>

#include <vector>

#include "../vmlib/vec2.hpp"
#include "../vmlib/vec3.hpp"

// struct SimpleMeshData
// {
// 	std::vector<Vec3f> positions;
// 	std::vector<Vec3f> normals;
// 	std::vector<Vec3f> colors;	  // For colored objects
// };

// Enhanced mesh structure supporting both colored and textured objects
struct SimpleMeshData
{
	std::vector<Vec3f> positions;
	std::vector<Vec3f> colors;	  // For colored objects
	std::vector<Vec2f> texcoords; // For textured objects
	std::vector<Vec3f> normals;	  // For lighting

	// Material type: 0 = colored, 1 = textured
	int materialType = 0;

	// Helper functions
	bool hasColors() const { return !colors.empty(); }
	bool hasTexcoords() const { return !texcoords.empty(); }
	bool hasNormals() const { return !normals.empty(); }

	// Get vertex count
	std::size_t vertexCount() const { return positions.size(); }

	// Clear all data
	void clear()
	{
		positions.clear();
		colors.clear();
		texcoords.clear();
		normals.clear();
	}
};

// For textured objects (positions + texcoords + normals)
struct TexturedMeshData
{
	std::vector<Vec3f> positions;
	std::vector<Vec2f> texcoords;
	std::vector<Vec3f> normals;
};

// For colored objects (positions + colors + normals)
struct ColoredMeshData
{
	std::vector<Vec3f> positions;
	std::vector<Vec3f> colors;
	std::vector<Vec3f> normals;
};

// SimpleMeshData concatenate( SimpleMeshData, SimpleMeshData const& );
// SimpleMeshData concatenate_many(std::initializer_list<std::reference_wrapper<const SimpleMeshData>> meshes);
SimpleMeshData make_cube_with_normals(Vec3f color);

// Creation functions
ColoredMeshData make_colored_cube(Vec3f color = {0.5f, 0.5f, 0.5f});
TexturedMeshData make_textured_plane();

// OBJ loading function that handles textures
SimpleMeshData load_wavefront_obj_with_mtl(char const *objPath, char const *mtlPath = nullptr);

// VAO creation functions
GLuint create_vao(SimpleMeshData const& mesh);

// GLuint create_vao_textured(TexturedMeshData const &mesh);
// GLuint create_vao_colored(ColoredMeshData const &mesh);

// GLuint create_vao_color(SimpleMeshDataColor const &);
// GLuint create_vao(SimpleMeshData const &);
// GLuint create_vao_from_meshes(std::initializer_list<std::reference_wrapper<const SimpleMeshData>> meshes);

#endif // SIMPLE_MESH_HPP_C6B749D6_C83B_434C_9E58_F05FC27FEFC9
