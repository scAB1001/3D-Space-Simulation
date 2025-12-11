#include "cone.hpp"

SimpleMeshData make_batched_indexed_cone(bool aCapped, std::size_t aSubdivs, Vec3f aColor, Mat44f aPreTransform)
{
    SimpleMeshData cone;

    // Pre-calculate base circle
    std::vector<Vec3f> baseCircle(aSubdivs);
    for (std::size_t i = 0; i < aSubdivs; ++i)
    {
        float angle = i / float(aSubdivs) * 2.f * std::numbers::pi_v<float>;
        baseCircle[i] = {0.f, std::cos(angle), std::sin(angle)};
    }

    // Setup transformation matrices
    Mat44f normalTransform = aPreTransform;
    normalTransform[0, 3] = 0.f;
    normalTransform[1, 3] = 0.f;
    normalTransform[2, 3] = 0.f;
    normalTransform[3, 3] = 1.f;

    // Transform base vertices
    std::vector<Vec3f> transformedBaseCircle(aSubdivs);
    std::vector<Vec3f> baseNormals(aSubdivs);

    for (std::size_t i = 0; i < aSubdivs; ++i)
    {
        Vec4f basePos4{baseCircle[i].x, baseCircle[i].y, baseCircle[i].z, 1.f};
        Vec4f transformedBase = aPreTransform * basePos4;
        transformedBase /= transformedBase.w;
        transformedBaseCircle[i] = Vec3f{transformedBase.x, transformedBase.y, transformedBase.z};

        // Base normal for shell
        Vec4f baseNormal4{0.f, baseCircle[i].y, baseCircle[i].z, 0.f};
        Vec4f transformedBaseNormal4 = normalTransform * baseNormal4;
        baseNormals[i] = normalize(Vec3f{transformedBaseNormal4.x,
                                         transformedBaseNormal4.y,
                                         transformedBaseNormal4.z});
    }

    // Transform tip
    Vec4f tip4{1.f, 0.f, 0.f, 1.f};
    Vec4f transformedTip4 = aPreTransform * tip4;
    transformedTip4 /= transformedTip4.w;
    Vec3f tip{transformedTip4.x, transformedTip4.y, transformedTip4.z};

    // Add base vertices
    for (std::size_t i = 0; i < aSubdivs; ++i)
    {
        cone.positions.push_back(transformedBaseCircle[i]);
        cone.colors.push_back(aColor);
        cone.normals.push_back(baseNormals[i]);
    }

    // Add tip vertex
    std::size_t tipIdx = cone.positions.size();
    cone.positions.push_back(tip);
    cone.colors.push_back(aColor);
    cone.normals.push_back(Vec3f{0.f, 0.f, 0.f});

    // Generate shell indices and compute smooth normals
    for (std::size_t i = 0; i < aSubdivs; ++i)
    {
        std::size_t next = (i + 1) % aSubdivs;

        // Triangle: base_i -> base_next -> tip
        cone.indices.push_back(i);
        cone.indices.push_back(next);
        cone.indices.push_back(tipIdx);

        // Compute face normal
        Vec3f v0 = cone.positions[i];
        Vec3f v1 = cone.positions[next];
        Vec3f v2 = cone.positions[tipIdx];

        Vec3f edge1 = v1 - v0;
        Vec3f edge2 = v2 - v0;
        Vec3f faceNormal = normalize(cross(edge1, edge2));

        // Accumulate normals for smooth shading
        cone.normals[i] += faceNormal;
        cone.normals[next] += faceNormal;
        cone.normals[tipIdx] += faceNormal;
    }

    // Base cap
    if (aCapped)
    {
        // Transform base center
        Vec4f center4{0.f, 0.f, 0.f, 1.f};
        Vec4f transformedCenter4 = aPreTransform * center4;
        transformedCenter4 /= transformedCenter4.w;
        Vec3f center{transformedCenter4.x, transformedCenter4.y, transformedCenter4.z};

        // Base cap normal
        Vec4f capNormal4{-1.f, 0.f, 0.f, 0.f};
        Vec4f transformedCapNormal4 = normalTransform * capNormal4;
        Vec3f capNormal = normalize(Vec3f{transformedCapNormal4.x,
                                          transformedCapNormal4.y,
                                          transformedCapNormal4.z});

        // Add base center vertex
        std::size_t centerIdx = cone.positions.size();
        cone.positions.push_back(center);
        cone.colors.push_back(aColor);
        cone.normals.push_back(capNormal);

        // Base cap indices (reverse winding for correct facing)
        for (std::size_t i = 0; i < aSubdivs; ++i)
        {
            std::size_t next = (i + 1) % aSubdivs;

            cone.indices.push_back(centerIdx);
            cone.indices.push_back(next);
            cone.indices.push_back(i);

            // Adjust base vertex normals to average between shell and cap
            cone.normals[i] += capNormal;
            cone.normals[next] += capNormal;
        }
    }

    // Normalize all accumulated normals
    for (auto &normal : cone.normals)
    {
        if (length(normal) > 0.f)
            normal = normalize(normal);
        else
            normal = Vec3f{0.f, 1.f, 0.f}; // Default up
    }

    return cone;
}