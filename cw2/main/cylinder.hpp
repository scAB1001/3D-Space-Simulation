#ifndef CYLINDER_HPP_E4D1E8EC_6CDA_4800_ABDD_264F643AF5DB
#define CYLINDER_HPP_E4D1E8EC_6CDA_4800_ABDD_264F643AF5DB

#include <vector>

#include <cstdlib>

#include "simple_mesh.hpp"

#include "../vmlib/vec3.hpp"
#include "../vmlib/mat33.hpp"
#include "../vmlib/mat44.hpp"

SimpleMeshData make_cylinder(
    bool aCapped = true,
    std::size_t aSubdivs = 16,
    Vec3f aColor = {1.f, 1.f, 1.f},
    Mat44f aPreTransform = kIdentity44f);

SimpleMeshData make_batched_indexed_cylinder(
    bool aCapped = true,
    std::size_t aSubdivs = 16,
    Vec3f aColor = {1.f, 1.f, 1.f},
    Mat44f aPreTransform = kIdentity44f);

SimpleMeshData make_cylinder_extended(
    bool aCapped = true,
    std::size_t aSubdivs = 16,
    std::size_t aLengthSubdivs = 1,
    Vec3f aColor = {1.f, 1.f, 1.f},
    Mat44f aPreTransform = kIdentity44f);

SimpleMeshData make_bent_cylinder(
    bool aCapped = true,
    std::size_t aSubdivs = 16,
    std::size_t aLengthSubdivs = 8,
    float aBendAngle = std::numbers::pi_v<float> / 2.f, // 90 degrees bend
    Vec3f aColor = {1.f, 1.f, 1.f},
    Mat44f aPreTransform = kIdentity44f);

#endif // CYLINDER_HPP_E4D1E8EC_6CDA_4800_ABDD_264F643AF5DB