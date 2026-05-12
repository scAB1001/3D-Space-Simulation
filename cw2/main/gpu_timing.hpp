#pragma once
#include <glad/glad.h>

constexpr int GPU_QUERY_BUFFER_SIZE = 8;

struct GpuTiming
{
    GLuint fullStart[GPU_QUERY_BUFFER_SIZE];
    GLuint fullEnd[GPU_QUERY_BUFFER_SIZE];
    int frameIndex = 0;
};

void initGpuTiming(GpuTiming &gt);
void destroyGpuTiming(GpuTiming &gt);
