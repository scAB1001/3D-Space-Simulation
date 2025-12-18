#ifndef SPACE_VEHICLE_HPP
#define SPACE_VEHICLE_HPP

#include "simple_mesh.hpp"

#include "cylinder.hpp"
#include "cone.hpp"
#include "cube.hpp"
#include "config.hpp"

#include "../vmlib/mat44.hpp"
#include "../vmlib/mat33.hpp"
#include "../vmlib/vec3.hpp"
#include "../vmlib/vec4.hpp"

namespace
{
    // Colours
    constexpr Vec3f bodyColor    {0.8f, 0.8f, 0.8f};
    constexpr Vec3f noseColor    {0.9f, 0.3f, 0.3f};
    constexpr Vec3f nozzleColor  {0.2f, 0.2f, 0.2f};
    constexpr Vec3f finColor     {0.3f, 0.6f, 0.9f};
    constexpr Vec3f cockpitColor {0.2f, 0.2f, 0.6f};

    // Dimensions
   constexpr float bodyRadius  = 1.5f;
   constexpr float bodyHeight  = 5.0f;        // rocket body height
   constexpr float noseHeight  = 2.0f;        // tip length
   constexpr float noseRadius  = bodyRadius * 1.0f;
   constexpr float baseHalfH   = 0.4f;        // base cube half-height
}

SimpleMeshData make_space_vehicle();

#endif // SPACE_VEHICLE_HPP