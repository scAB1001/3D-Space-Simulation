#include "gpu_timing.hpp"

void initGpuTiming(GpuTiming &gt)
{
    glGenQueries(GPU_QUERY_BUFFER_SIZE, gt.fullStart);
    glGenQueries(GPU_QUERY_BUFFER_SIZE, gt.fullEnd);
    gt.frameIndex = 0;
}

void destroyGpuTiming(GpuTiming &gt)
{
    glDeleteQueries(GPU_QUERY_BUFFER_SIZE, gt.fullStart);
    glDeleteQueries(GPU_QUERY_BUFFER_SIZE, gt.fullEnd);
}
