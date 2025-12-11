#include "space_vehicle.hpp"

#include "cube.hpp"
#include "cylinder.hpp"
#include "cone.hpp"
#include "config.hpp"

#include "../vmlib/mat44.hpp"
#include "../vmlib/mat33.hpp"
#include "../vmlib/vec3.hpp"
#include "../vmlib/vec4.hpp"

namespace
{
    constexpr Vec3f bodyColor    {0.8f, 0.8f, 0.8f};
    constexpr Vec3f noseColor    {0.9f, 0.3f, 0.3f};
    constexpr Vec3f nozzleColor  {0.2f, 0.2f, 0.2f};
    constexpr Vec3f finColor     {0.3f, 0.6f, 0.9f};
    constexpr Vec3f cockpitColor {0.2f, 0.2f, 0.6f};

    // Append src into dst, transforming positions by M.
    // Normals are recomputed at the end, so we ignore src.normals.
    void append_transformed_mesh(SimpleMeshData& dst,
                                 SimpleMeshData const& src,
                                 Mat44f const& M)
    {
        std::size_t baseIndex = dst.positions.size();

        // Transform positions
        for (std::size_t i = 0; i < src.positions.size(); ++i)
        {
            Vec3f p  = src.positions[i];
            Vec4f p4 = M * Vec4f{p.x, p.y, p.z, 1.f};
            dst.positions.push_back(Vec3f{p4.x, p4.y, p4.z});

            // Carry over colors if present (we’re using coloured meshes)
            if (src.hasColors())
                dst.colors.push_back(src.colors[i]);

            // Copy texcoords if you ever build textured vehicle parts
            if (src.hasTexcoords())
                dst.texcoords.push_back(src.texcoords[i]);
        }

        // Indices
        if (src.hasIndices())
        {
            for (auto idx : src.indices)
                dst.indices.push_back(static_cast<unsigned int>(baseIndex + idx));
        }
        else
        {
            // Generate trivial indices if src is non-indexed
            for (std::size_t i = 0; i < src.positions.size(); ++i)
                dst.indices.push_back(static_cast<unsigned int>(baseIndex + i));
        }
    }
}SimpleMeshData make_space_vehicle()
{
    SimpleMeshData vehicle;

    // 1. Meshes
    SimpleMeshData bodyRaw   = make_cylinder(true, 32, bodyColor);   // axis +X
    SimpleMeshData noseRaw   = make_cone(true, 32, noseColor);       // axis +X
    SimpleMeshData fin       = make_indexed_cube(finColor);
    SimpleMeshData base      = make_indexed_cube(cockpitColor);

    bodyRaw.materialType = 0;
    noseRaw.materialType = 0;
    fin.materialType     = 0;
    base.materialType    = 0;

    // ===== DIMENSIONS =====
    float bodyRadius  = 1.5f;
    float bodyHeight  = 5.0f;        // rocket body height
    float noseHeight  = 2.0f;        // tip length
    float noseRadius  = bodyRadius * 1.0f;
    float baseHalfH   = 0.4f;        // base cube half-height

    // ===== BODY: axis +X -> +Y, base at y=0, top at y=bodyHeight =====
    // local cylinder: x in [0,1], radius 1 in y/z
    Mat44f S_body = make_scaling(bodyHeight, bodyRadius, bodyRadius);   // length on X
    Mat44f R_body = make_rotation_z(+Config::kFloatPi * 0.5f);          // X -> Y
    Mat44f T_body = make_translation(Vec3f{0.f, 0.f, 0.f});             // no offset
    Mat44f M_body = T_body * R_body * S_body;                           // y ∈ [0, bodyHeight]

    // ===== CONE: axis +X -> +Y, base at cylinder top y=bodyHeight =====
    // local cone: base at x=0, tip at x=1
    Mat44f S_nose = make_scaling(noseHeight, noseRadius, noseRadius);   // length on X
    Mat44f R_nose = make_rotation_z(+Config::kFloatPi * 0.5f);          // X -> Y
    Mat44f T_nose = make_translation(Vec3f{0.f, bodyHeight, 0.f});      // base at y=bodyHeight
    Mat44f M_nose = T_nose * R_nose * S_nose;                           // tip at y=bodyHeight + noseHeight

    // ===== BASE CUBE: top at y = 0, bottom below pad =====
    Mat44f S_base = make_scaling(0.6, (0.4*3.9), 0.6);
    Mat44f T_base = make_translation(Vec3f{0.f, -baseHalfH, 0.f});      // y ∈ [-baseH*2, 0]
    Mat44f M_base = T_base * S_base;

    // ===== FINS: attached partway up the body =====
    Mat44f S_fin = make_scaling(0.15f, 1.6f, 1.1f);
    Mat44f R_fin = make_rotation_z(Config::kFloatPi + 0.4f);
    Mat44f T_fin = make_translation(Vec3f{bodyRadius + 0.1f, 0.3f * bodyHeight, 0.f});
    Mat44f M_fin_base = T_fin * R_fin * S_fin;

    // 3. Append meshes
    append_transformed_mesh(vehicle, base,   M_base);
    append_transformed_mesh(vehicle, bodyRaw, M_body);
    append_transformed_mesh(vehicle, noseRaw, M_nose);

    append_transformed_mesh(vehicle, fin, M_fin_base);
    Mat44f R90  = make_rotation_y(Config::kFloatPi * 0.5f);
    Mat44f R180 = make_rotation_y(Config::kFloatPi);
    Mat44f R270 = make_rotation_y(Config::kFloatPi * 1.5f);
    append_transformed_mesh(vehicle, fin, R90  * M_fin_base);
    append_transformed_mesh(vehicle, fin, R180 * M_fin_base);
    append_transformed_mesh(vehicle, fin, R270 * M_fin_base);

    vehicle.materialType = 0;
    vehicle.calculate_normals();

    return vehicle;
}
