#ifndef LOADOBJ_HPP
#define LOADOBJ_HPP

#include "simple_mesh.hpp"

// Material structure for MTL files
struct Material
{
    std::string name;
    Vec3f ambient;
    Vec3f diffuse;
    Vec3f specular;
    float shininess;
    float alpha; // transparency
};

SimpleMeshData load_wavefront_obj( char const* aPath );

// Enhanced OBJ loading that handles materials
SimpleMeshData load_wavefront_obj_with_mtl(char const *objPath, std::vector<Material> &materials);

#endif // LOADOBJ_HPP
