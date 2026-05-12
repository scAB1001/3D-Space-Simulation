#ifndef CONE_HPP
#define CONE_HPP

#include "simple_mesh.hpp"

SimpleMeshData make_batched_indexed_cone(
    bool aCapped = true,
    std::size_t aSubdivs = 16,
    Vec3f aColor = {1.f, 1.f, 1.f},
    Mat44f aPreTransform = kIdentity44f);

#endif // CONE_HPP