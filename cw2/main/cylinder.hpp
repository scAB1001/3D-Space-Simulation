#ifndef CYLINDER_HPP
#define CYLINDER_HPP

#include "simple_mesh.hpp"

SimpleMeshData make_batched_indexed_cylinder(
    bool aCapped = true,
    std::size_t aSubdivs = 16,
    Vec3f aColor = {1.f, 1.f, 1.f},
    Mat44f aPreTransform = kIdentity44f);

#endif // CYLINDER_HPP