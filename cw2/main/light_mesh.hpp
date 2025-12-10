#pragma once
#include "simple_mesh.hpp"

inline SimpleMeshData make_light_sphere(float r, Vec3f color)
{
    SimpleMeshData m;

    const int steps = 16;

    for (int i = 0; i <= steps; ++i)
    {
        float v = float(i) / steps;
        float phi = v * Config::kFloatPi;

        for (int j = 0; j <= steps; ++j)
        {
            float u = float(j) / steps;
            float theta = u * 2.f * Config::kFloatPi;

            float x = r * sin(phi) * cos(theta);
            float y = r * cos(phi);
            float z = r * sin(phi) * sin(theta);

            m.positions.emplace_back(x, y, z);
            m.colors.emplace_back(color);
        }
    }

    for (int i = 0; i < steps; ++i)
    {
        for (int j = 0; j < steps; ++j)
        {
            int a = i * (steps + 1) + j;
            int b = a + 1;
            int c = a + (steps + 1);
            int d = c + 1;

            m.indices.push_back(a);
            m.indices.push_back(c);
            m.indices.push_back(b);

            m.indices.push_back(b);
            m.indices.push_back(c);
            m.indices.push_back(d);
        }
    }

    m.materialType = 0;
    m.calculate_normals();

    return m;
}
