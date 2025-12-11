#ifndef PERFORMANCE_HPP
#define PERFORMANCE_HPP

#include <chrono>
#include <vector>
#include <string>
#include <print>
#include <glad/glad.h>

// Enable/disable performance measurements
// To enable: compile with -DENABLE_PERFORMANCE_MEASUREMENT
// #ifdef ENABLE_PERFORMANCE_MEASUREMENT
//     #define PERFORMANCE_ENABLED 1
// #else
//     #define PERFORMANCE_ENABLED 0
// #endif

namespace Performance {

using Clock = std::chrono::high_resolution_clock;
using Nanoseconds = std::chrono::nanoseconds;

// Query labels for different measurement sections
enum class QueryLabel {
    FRAME_TOTAL,        // Complete frame rendering (excluding swap buffers)
    SECTION_1_2,        // Section 1.2: Terrain rendering
    SECTION_1_4,        // Section 1.4: Landing pads rendering
    SECTION_1_5,        // Section 1.5: Space vehicle rendering
    COUNT               // Total number of query labels
};

// Single frame timing data
struct FrameTiming {
    // GPU timings (nanoseconds)
    uint64_t gpuTotalTime = 0;
    uint64_t gpuSection12 = 0;
    uint64_t gpuSection14 = 0;
    uint64_t gpuSection15 = 0;

    // CPU timings (nanoseconds)
    Nanoseconds cpuSubmitTime = Nanoseconds::zero();
    Nanoseconds cpuFrameTime = Nanoseconds::zero();

    // Frame number for reference
    uint64_t frameNumber = 0;

    // Validation flags
    bool gpuDataValid = false;
    bool cpuDataValid = false;
};

// Performance measurement manager
class PerformanceMonitor {
private:
    // Query objects for GPU timing
    GLuint queryObjects[static_cast<size_t>(QueryLabel::COUNT)];
    bool queriesInitialized = false;

    // Current frame state
    bool measuringFrame = false;
    QueryLabel currentQuery = QueryLabel::COUNT;

    // Timing data storage
    std::vector<FrameTiming> frameTimings;
    FrameTiming currentFrame;

    // CPU timing
    Clock::time_point frameStartTime;
    Clock::time_point submitStartTime;
    uint64_t frameCounter = 0;

    // Statistics
    bool statsCalculated = false;
    struct Stats {
        uint64_t minGPUTime = UINT64_MAX;
        uint64_t maxGPUTime = 0;
        uint64_t avgGPUTime = 0;
        uint64_t totalGPUTime = 0;

        Nanoseconds minCPUTime = Nanoseconds::max();
        Nanoseconds maxCPUTime = Nanoseconds::zero();
        Nanoseconds avgCPUTime = Nanoseconds::zero();
        Nanoseconds totalCPUTime = Nanoseconds::zero();

        size_t validFrames = 0;
    } statistics;

public:
    PerformanceMonitor();
    ~PerformanceMonitor();

    // Initialize query objects
    bool initialize();

    // Frame measurement control
    void beginFrame();
    void endFrame();

    // GPU query control
    void beginGPUQuery(QueryLabel label);
    void endGPUQuery(QueryLabel label);

    // CPU timing
    void beginCPUSubmitMeasurement();
    void endCPUSubmitMeasurement();

    // Get timing data
    const FrameTiming& getCurrentFrameTiming() const { return currentFrame; }
    const std::vector<FrameTiming>& getAllFrameTimings() const { return frameTimings; }

    // Statistics
    void calculateStatistics();
    void printStatistics() const;
    void printCurrentFrame() const;

    // Save to file (optional)
    bool saveToCSV(const std::string& filename) const;

private:
    // Helper methods
    uint64_t getQueryResult(GLuint queryId) const;
    void resetCurrentFrame();
    void validateAndStoreFrame();

    // Disable copying
    PerformanceMonitor(const PerformanceMonitor&) = delete;
    PerformanceMonitor& operator=(const PerformanceMonitor&) = delete;
};

// Singleton accessor (optional, but convenient)
PerformanceMonitor& getMonitor();

} // namespace Performance

#endif // PERFORMANCE_HPP