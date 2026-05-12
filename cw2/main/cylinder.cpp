#include "cylinder.hpp"

SimpleMeshData make_batched_indexed_cylinder(bool aCapped, std::size_t aSubdivs, Vec3f aColor, Mat44f aPreTransform)
{
    /* References for code inspiration:
     * - https://learnopengl.com/Model-Loading/Mesh
     * - https://www.gamedev.net/articles/programming/graphics/opengl-batch-rendering-r3900/
     * - https://github.com/robmaier/menderer
     * - https://www.opengl-tutorial.org/intermediate-tutorials/tutorial-9-vbo-indexing/
     */
    
    SimpleMeshData cylinder;

    // Pre-calculate circle points
    std::vector<Vec3f> baseCircle(aSubdivs);
    for (std::size_t i = 0; i < aSubdivs; ++i)
    {
        float angle = i / float(aSubdivs) * 2.f * std::numbers::pi_v<float>;
        baseCircle[i] = {0.f, std::cos(angle), std::sin(angle)};
    }

    // Apply pre-transform to create base circle positions
    std::vector<Vec3f> transformedBaseCircle(aSubdivs);
    std::vector<Vec3f> transformedTopCircle(aSubdivs);

    Mat44f normalTransform = aPreTransform;
    // Remove translation for normal matrix
    normalTransform[0, 3] = 0.f;
    normalTransform[1, 3] = 0.f;
    normalTransform[2, 3] = 0.f;
    normalTransform[3, 3] = 1.f;

    // Transform all positions and compute normals
    for (std::size_t i = 0; i < aSubdivs; ++i)
    {
        // Bottom ring
        Vec4f bottomPos4{baseCircle[i].x, baseCircle[i].y, baseCircle[i].z, 1.f};
        Vec4f transformedBottom = aPreTransform * bottomPos4;
        transformedBottom /= transformedBottom.w;
        transformedBaseCircle[i] = Vec3f{transformedBottom.x, transformedBottom.y, transformedBottom.z};

        // Top ring
        Vec4f topPos4{1.f, baseCircle[i].y, baseCircle[i].z, 1.f};
        Vec4f transformedTop = aPreTransform * topPos4;
        transformedTop /= transformedTop.w;
        transformedTopCircle[i] = Vec3f{transformedTop.x, transformedTop.y, transformedTop.z};

        // Shell normal (same for both rings at this vertex)
        Vec4f normal4{0.f, baseCircle[i].y, baseCircle[i].z, 0.f};
        Vec4f transformedNormal4 = normalTransform * normal4;
        Vec3f shellNormal = normalize(Vec3f{transformedNormal4.x, transformedNormal4.y, transformedNormal4.z});

        // Add bottom vertex
        cylinder.positions.push_back(transformedBaseCircle[i]);
        cylinder.colors.push_back(aColor);
        cylinder.normals.push_back(shellNormal);

        // Add top vertex
        cylinder.positions.push_back(transformedTopCircle[i]);
        cylinder.colors.push_back(aColor);
        cylinder.normals.push_back(shellNormal);
    }

    // Generate shell indices
    for (std::size_t i = 0; i < aSubdivs; ++i)
    {
        std::size_t next = (i + 1) % aSubdivs;
        std::size_t bottom_i = i * 2;
        std::size_t top_i = i * 2 + 1;
        std::size_t bottom_next = next * 2;
        std::size_t top_next = next * 2 + 1;

        // Quad as two triangles
        cylinder.indices.push_back(bottom_i);
        cylinder.indices.push_back(bottom_next);
        cylinder.indices.push_back(top_i);

        cylinder.indices.push_back(bottom_next);
        cylinder.indices.push_back(top_next);
        cylinder.indices.push_back(top_i);
    }

    // Add caps
    if (aCapped)
    {
        // Bottom center
        Vec4f bottomCenter4{0.f, 0.f, 0.f, 1.f};
        Vec4f transformedBottomCenter = aPreTransform * bottomCenter4;
        transformedBottomCenter /= transformedBottomCenter.w;
        Vec3f bottomCenter{transformedBottomCenter.x, transformedBottomCenter.y, transformedBottomCenter.z};

        // Bottom cap normal
        Vec4f bottomNormal4{-1.f, 0.f, 0.f, 0.f};
        Vec4f transformedBottomNormal4 = normalTransform * bottomNormal4;
        Vec3f bottomCapNormal = normalize(Vec3f{transformedBottomNormal4.x,
                                                transformedBottomNormal4.y,
                                                transformedBottomNormal4.z});

        // Add bottom center vertex
        std::size_t bottomCenterIdx = cylinder.positions.size();
        cylinder.positions.push_back(bottomCenter);
        cylinder.colors.push_back(aColor);
        cylinder.normals.push_back(bottomCapNormal);

        // Bottom cap indices
        for (std::size_t i = 0; i < aSubdivs; ++i)
        {
            std::size_t next = (i + 1) % aSubdivs;
            std::size_t bottom_i = i * 2; // Bottom ring vertex

            cylinder.indices.push_back(bottomCenterIdx);
            cylinder.indices.push_back(bottom_i);
            cylinder.indices.push_back(next * 2); // bottom_next
        }

        // Top center
        Vec4f topCenter4{1.f, 0.f, 0.f, 1.f};
        Vec4f transformedTopCenter = aPreTransform * topCenter4;
        transformedTopCenter /= transformedTopCenter.w;
        Vec3f topCenter{transformedTopCenter.x, transformedTopCenter.y, transformedTopCenter.z};

        // Top cap normal
        Vec4f topNormal4{1.f, 0.f, 0.f, 0.f};
        Vec4f transformedTopNormal4 = normalTransform * topNormal4;
        Vec3f topCapNormal = normalize(Vec3f{transformedTopNormal4.x,
                                             transformedTopNormal4.y,
                                             transformedTopNormal4.z});

        // Add top center vertex
        std::size_t topCenterIdx = cylinder.positions.size();
        cylinder.positions.push_back(topCenter);
        cylinder.colors.push_back(aColor);
        cylinder.normals.push_back(topCapNormal);

        // Top cap indices (reverse winding for correct facing)
        for (std::size_t i = 0; i < aSubdivs; ++i)
        {
            std::size_t next = (i + 1) % aSubdivs;
            std::size_t top_i = i * 2 + 1; // Top ring vertex

            cylinder.indices.push_back(topCenterIdx);
            cylinder.indices.push_back(next * 2 + 1); // top_next
            cylinder.indices.push_back(top_i);
        }
    }

    return cylinder;
}
