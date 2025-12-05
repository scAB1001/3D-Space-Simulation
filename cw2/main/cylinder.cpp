#include "cylinder.hpp"

#include <numbers>

SimpleMeshData make_cylinder(bool aCapped, std::size_t aSubdivs, Vec3f aColor, Mat44f aPreTransform)
{
    // Calculate expected vertex count
    std::size_t shellVertices = aSubdivs * 6;
    std::size_t capVertices = aCapped ? (aSubdivs * 3 * 2) : 0;
    std::size_t expectedVertices = shellVertices + capVertices;

    // Pre-allocate vectors
    std::vector<Vec3f> pos(expectedVertices);
    std::vector<Vec3f> norms(expectedVertices);
    std::vector<Vec3f> colors(expectedVertices, aColor);

    // Pre-compute circle points
    std::vector<Vec4f> circlePoints(aSubdivs);
    std::vector<Vec3f> circleNormals(aSubdivs);
    for (std::size_t i = 0; i < aSubdivs; ++i)
    {
        float angle = i / float(aSubdivs) * 2.f * std::numbers::pi_v<float>;
        float y = std::cos(angle);
        float z = std::sin(angle);
        circlePoints[i] = {0.f, y, z, 1.f};
        circleNormals[i] = normalize(Vec3f{0.f, y, z});
    }

    // Generate cylinder shell
    for (std::size_t i = 0, idx = 0; i < aSubdivs; ++i)
    {
        std::size_t next = (i + 1) % aSubdivs;

        // Transform points
        Vec4f p0_start = aPreTransform * circlePoints[i];
        Vec4f p1_start = aPreTransform * circlePoints[next];
        Vec4f p0_end = aPreTransform * Vec4f{1.f, circlePoints[i].y, circlePoints[i].z, 1.f};
        Vec4f p1_end = aPreTransform * Vec4f{1.f, circlePoints[next].y, circlePoints[next].z, 1.f};

        p0_start /= p0_start.w;
        p1_start /= p1_start.w;
        p0_end /= p0_end.w;
        p1_end /= p1_end.w;

        // Store positions
        pos[idx] = {p0_start.x, p0_start.y, p0_start.z};
        pos[idx + 1] = {p1_start.x, p1_start.y, p1_start.z};
        pos[idx + 2] = {p0_end.x, p0_end.y, p0_end.z};
        pos[idx + 3] = {p1_start.x, p1_start.y, p1_start.z};
        pos[idx + 4] = {p1_end.x, p1_end.y, p1_end.z};
        pos[idx + 5] = {p0_end.x, p0_end.y, p0_end.z};

        // Store normals
        norms[idx] = circleNormals[i];
        norms[idx + 1] = circleNormals[next];
        norms[idx + 2] = circleNormals[i];
        norms[idx + 3] = circleNormals[next];
        norms[idx + 4] = circleNormals[next];
        norms[idx + 5] = circleNormals[i];

        idx += 6;
    }

    // Caps
    if (aCapped)
    {
        std::size_t baseIdx = shellVertices;

        // Start cap
        Vec4f startCenter = aPreTransform * Vec4f{0.f, 0.f, 0.f, 1.f};
        startCenter /= startCenter.w;
        Vec3f startNormal{-1.f, 0.f, 0.f};

        for (std::size_t i = 0; i < aSubdivs; ++i)
        {
            std::size_t next = (i + 1) % aSubdivs;

            Vec4f p0 = aPreTransform * circlePoints[i];
            Vec4f p1 = aPreTransform * circlePoints[next];
            p0 /= p0.w;
            p1 /= p1.w;

            pos[baseIdx] = {startCenter.x, startCenter.y, startCenter.z};
            pos[baseIdx + 1] = {p0.x, p0.y, p0.z};
            pos[baseIdx + 2] = {p1.x, p1.y, p1.z};

            norms[baseIdx] = startNormal;
            norms[baseIdx + 1] = startNormal;
            norms[baseIdx + 2] = startNormal;

            baseIdx += 3;
        }

        // End cap
        Vec4f endCenter = aPreTransform * Vec4f{1.f, 0.f, 0.f, 1.f};
        endCenter /= endCenter.w;
        Vec3f endNormal{1.f, 0.f, 0.f};

        for (std::size_t i = 0; i < aSubdivs; ++i)
        {
            std::size_t next = (i + 1) % aSubdivs;

            Vec4f p0_raw = Vec4f{1.f, circlePoints[i].y, circlePoints[i].z, 1.f};
            Vec4f p1_raw = Vec4f{1.f, circlePoints[next].y, circlePoints[next].z, 1.f};

            Vec4f p0 = aPreTransform * p0_raw;
            Vec4f p1 = aPreTransform * p1_raw;
            p0 /= p0.w;
            p1 /= p1.w;

            pos[baseIdx] = {endCenter.x, endCenter.y, endCenter.z};
            pos[baseIdx + 1] = {p1.x, p1.y, p1.z};
            pos[baseIdx + 2] = {p0.x, p0.y, p0.z};

            norms[baseIdx] = endNormal;
            norms[baseIdx + 1] = endNormal;
            norms[baseIdx + 2] = endNormal;

            baseIdx += 3;
        }
    }

    // FIXED: Actually return the data
    return SimpleMeshData{std::move(pos), std::move(colors), {}, std::move(norms)};
}

SimpleMeshData make_batched_indexed_cylinder(bool aCapped, std::size_t aSubdivs, Vec3f aColor, Mat44f aPreTransform)
{
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

    // Add caps if requested
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

SimpleMeshData make_cylinder_extended(bool aCapped, std::size_t aSubdivs, std::size_t aLengthSubdivs, Vec3f aColor, Mat44f aPreTransform)
{
    std::vector<Vec3f> pos;

    // Generate vertices along the length
    for (std::size_t lengthIdx = 0; lengthIdx < aLengthSubdivs; ++lengthIdx)
    {
        float x0 = float(lengthIdx) / float(aLengthSubdivs);
        float x1 = float(lengthIdx + 1) / float(aLengthSubdivs);

        // Generate circle for this segment
        float prevY = std::cos(0.f);
        float prevZ = std::sin(0.f);

        for (std::size_t i = 0; i < aSubdivs; ++i)
        {
            float const angle = (i + 1) / float(aSubdivs) * 2.f * std::numbers::pi_v<float>;
            float y = std::cos(angle);
            float z = std::sin(angle);

            // Two triangles for this segment
            // Triangle 1
            pos.emplace_back(Vec3f{x0, prevY, prevZ});
            pos.emplace_back(Vec3f{x0, y, z});
            pos.emplace_back(Vec3f{x1, prevY, prevZ});

            // Triangle 2
            pos.emplace_back(Vec3f{x0, y, z});
            pos.emplace_back(Vec3f{x1, y, z});
            pos.emplace_back(Vec3f{x1, prevY, prevZ});

            prevY = y;
            prevZ = z;
        }
    }

    // Caps (same as before)
    if (aCapped)
    {
        // Start cap
        float prevY = std::cos(0.f);
        float prevZ = std::sin(0.f);
        Vec3f startCenter{0.f, 0.f, 0.f};

        for (std::size_t i = 0; i < aSubdivs; ++i)
        {
            float const angle = (i + 1) / float(aSubdivs) * 2.f * std::numbers::pi_v<float>;
            float y = std::cos(angle);
            float z = std::sin(angle);

            pos.emplace_back(startCenter);
            pos.emplace_back(Vec3f{0.f, prevY, prevZ});
            pos.emplace_back(Vec3f{0.f, y, z});

            prevY = y;
            prevZ = z;
        }

        // End cap
        prevY = std::cos(0.f);
        prevZ = std::sin(0.f);
        Vec3f endCenter{1.f, 0.f, 0.f};

        for (std::size_t i = 0; i < aSubdivs; ++i)
        {
            float const angle = (i + 1) / float(aSubdivs) * 2.f * std::numbers::pi_v<float>;
            float y = std::cos(angle);
            float z = std::sin(angle);

            pos.emplace_back(endCenter);
            pos.emplace_back(Vec3f{1.f, y, z});
            pos.emplace_back(Vec3f{1.f, prevY, prevZ});

            prevY = y;
            prevZ = z;
        }
    }

    // Apply pre-transform
    for (auto &p : pos)
    {
        Vec4f p4{p.x, p.y, p.z, 1.f};
        Vec4f t = aPreTransform * p4;
        t /= t.w;
        p = Vec3f{t.x, t.y, t.z};
    }

    std::vector<Vec3f> col(pos.size(), aColor);
    return SimpleMeshData{std::move(pos), std::move(col)};
}

SimpleMeshData make_bent_cylinder(bool aCapped, std::size_t aSubdivs, std::size_t aLengthSubdivs, float aBendAngle, Vec3f aColor, Mat44f aPreTransform)
    {
        std::vector<Vec3f> pos;

        // Calculate bend radius and other parameters
        float totalLength = 1.0f;                    // Base length
        float bendRadius = totalLength / aBendAngle; // Arc length = radius * angle

        for (std::size_t lengthIdx = 0; lengthIdx < aLengthSubdivs; ++lengthIdx)
        {
            float t0 = float(lengthIdx) / float(aLengthSubdivs);
            float t1 = float(lengthIdx + 1) / float(aLengthSubdivs);

            // Calculate bend parameters
            float angle0 = t0 * aBendAngle;
            float angle1 = t1 * aBendAngle;

            // Generate circle for this segment (bent along arc)
            float prevY = std::cos(0.f);
            float prevZ = std::sin(0.f);

            for (std::size_t i = 0; i < aSubdivs; ++i)
            {
                float const circleAngle = (i + 1) / float(aSubdivs) * 2.f * std::numbers::pi_v<float>;
                float y = std::cos(circleAngle);
                float z = std::sin(circleAngle);

                // Calculate pos along bent path
                Vec3f p0_prev{
                    bendRadius * std::sin(angle0),
                    prevY * 0.1f, // Scale radius
                    bendRadius * (1.f - std::cos(angle0)) + prevZ * 0.1f};

                Vec3f p0_current{
                    bendRadius * std::sin(angle0),
                    y * 0.1f,
                    bendRadius * (1.f - std::cos(angle0)) + z * 0.1f};

                Vec3f p1_prev{
                    bendRadius * std::sin(angle1),
                    prevY * 0.1f,
                    bendRadius * (1.f - std::cos(angle1)) + prevZ * 0.1f};

                Vec3f p1_current{
                    bendRadius * std::sin(angle1),
                    y * 0.1f,
                    bendRadius * (1.f - std::cos(angle1)) + z * 0.1f};

                // Two triangles for this segment
                // Triangle 1
                pos.emplace_back(p0_prev);
                pos.emplace_back(p0_current);
                pos.emplace_back(p1_prev);

                // Triangle 2
                pos.emplace_back(p0_current);
                pos.emplace_back(p1_current);
                pos.emplace_back(p1_prev);

                prevY = y;
                prevZ = z;
            }
        }

        // TODO: Add bent caps (more complex)

        // Apply pre-transform
        for (auto &p : pos)
        {
            Vec4f p4{p.x, p.y, p.z, 1.f};
            Vec4f t = aPreTransform * p4;
            t /= t.w;
            p = Vec3f{t.x, t.y, t.z};
        }

        std::vector<Vec3f> col(pos.size(), aColor);
        return SimpleMeshData{std::move(pos), std::move(col)};
    }