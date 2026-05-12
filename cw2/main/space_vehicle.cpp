#include "space_vehicle.hpp"

SimpleMeshData make_space_vehicle()
{
    SimpleMeshData vehicle;

    // Meshes templates
    SimpleMeshData base      = make_indexed_cube(cockpitColor);
    SimpleMeshData bodyRaw   = make_batched_indexed_cylinder(true, 32, bodyColor);
    SimpleMeshData noseRaw   = make_batched_indexed_cone(true, 32, noseColor);
    SimpleMeshData fin       = make_indexed_cube(finColor);

    // Set material types
    base.materialType    = 0;
    bodyRaw.materialType = 0;
    noseRaw.materialType = 0;
    fin.materialType     = 0;

    // Base cuboid: top at y = 0, bottom below pad
    Mat44f S_base = make_scaling(0.6, (0.4 * 3.9), 0.6);
    Mat44f T_base = make_translation(Vec3f{0.f, -baseHalfH, 0.f});
    Mat44f M_base = T_base * S_base;
    apply_transform_to_mesh(base, M_base);

    // Body cylinder: axis +X -> +Y, base at y=0, top at y=bodyHeight
    Mat44f S_body = make_scaling(bodyHeight, bodyRadius, bodyRadius);
    Mat44f R_body = make_rotation_z(+Config::kFloatPi * 0.5f);
    Mat44f T_body = make_translation(Config::kZeroVec3);
    Mat44f M_body = T_body * R_body * S_body;
    apply_transform_to_mesh(bodyRaw, M_body);

    // Nose cone: axis +X -> +Y, base at cylinder top y=bodyHeight
    Mat44f S_nose = make_scaling(noseHeight, noseRadius, noseRadius);
    Mat44f R_nose = make_rotation_z(+Config::kFloatPi * 0.5f);
    Mat44f T_nose = make_translation(Vec3f{0.f, bodyHeight, 0.f});
    Mat44f M_nose = T_nose * R_nose * S_nose;
    apply_transform_to_mesh(noseRaw, M_nose);

    // Fin cuboids: attached partway up the body
    Mat44f S_fin = make_scaling(0.15f, 1.6f, 1.1f);
    Mat44f R_fin = make_rotation_z(Config::kFloatPi + 0.4f);
    Mat44f T_fin = make_translation(Vec3f{bodyRadius + 0.1f, 0.3f * bodyHeight, 0.f});
    Mat44f M_fin_base = T_fin * R_fin * S_fin;
    apply_transform_to_mesh(fin, M_fin_base);

    // Transform to append fins 2, 3 and 4 around the body
    Mat44f R90  = make_rotation_y(Config::kFloatPi * 0.5f);
    Mat44f R180 = make_rotation_y(Config::kFloatPi);
    Mat44f R270 = make_rotation_y(Config::kFloatPi * 1.5f);

    // Create transformed copies of fin
    SimpleMeshData fin2 = fin;
    apply_transform_to_mesh(fin2, R90);
    SimpleMeshData fin3 = fin;
    apply_transform_to_mesh(fin3, R180);
    SimpleMeshData fin4 = fin;
    apply_transform_to_mesh(fin4, R270);

    // Use concatenate_many for efficiency
    vehicle = concatenate_many({
        std::cref(base),
        std::cref(bodyRaw),
        std::cref(noseRaw),
        std::cref(fin),
        std::cref(fin2),
        std::cref(fin3),
        std::cref(fin4)
    });

    vehicle.materialType = 0;
    vehicle.calculate_normals();

    return vehicle;
}
