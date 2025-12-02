#ifndef SIMPLE_MESH_HPP_C6B749D6_C83B_434C_9E58_F05FC27FEFC9
#define SIMPLE_MESH_HPP_C6B749D6_C83B_434C_9E58_F05FC27FEFC9

#include <glad/glad.h>

#include <vector>

#include "../vmlib/vec3.hpp"
#include "../vmlib/vec4.hpp"
#include "../vmlib/mat44.hpp"

struct SimpleMeshData
{
	std::vector<Vec3f> positions;
	std::vector<Vec3f> colors;
	std::vector<Vec3f> normals;
};

SimpleMeshData create_colored_arrow(const SimpleMeshData &baseArrow, Vec3f color, Mat44f transform);

SimpleMeshData concatenate(SimpleMeshData, SimpleMeshData const &);

SimpleMeshData concatenate_many(std::initializer_list<std::reference_wrapper<const SimpleMeshData>> meshes);

SimpleMeshData make_cube_with_normals(Vec3f color);

GLuint create_vao(SimpleMeshData const &);

GLuint create_vao_from_meshes(std::initializer_list<std::reference_wrapper<const SimpleMeshData>> meshes);

#endif // SIMPLE_MESH_HPP_C6B749D6_C83B_434C_9E58_F05FC27FEFC9
