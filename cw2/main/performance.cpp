#include "performance.hpp"

namespace Performance {

PerformanceMonitor::PerformanceMonitor() {
    std::fill(std::begin(queryObjects), std::end(queryObjects), 0);
}

PerformanceMonitor::~PerformanceMonitor() {
    if (queriesInitialized) {
        glDeleteQueries(static_cast<GLsizei>(std::size(queryObjects)), queryObjects);
    }
}

bool PerformanceMonitor::initialize() {
// #if PERFORMANCE_ENABLED
    // Check if timer queries are supported
    GLint timestampBits = 0;
    glGetIntegerv(GL_TIMESTAMP, &timestampBits);

    if (timestampBits == 0) {
        std::print(stderr, "PerformanceMonitor: GPU timestamp queries not supported\n");
        return false;
    }

    // Generate query objects
    glGenQueries(static_cast<GLsizei>(std::size(queryObjects)), queryObjects);

    // Verify query creation
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::print(stderr, "PerformanceMonitor: Failed to create query objects: {}\n", error);
        return false;
    }

    queriesInitialized = true;
    std::print("PerformanceMonitor: Initialized with {} query objects\n",
               static_cast<size_t>(QueryLabel::COUNT));
    return true;
// #else
//     return false;
// #endif
}

void PerformanceMonitor::beginFrame() {
// #if PERFORMANCE_ENABLED
    if (!queriesInitialized) return;

    measuringFrame = true;
    frameCounter++;

    // Reset current frame data
    resetCurrentFrame();
    currentFrame.frameNumber = frameCounter;

    // Start CPU frame timing
    frameStartTime = Clock::now();

    // Begin total frame GPU measurement
    beginGPUQuery(QueryLabel::FRAME_TOTAL);
// #endif
}

void PerformanceMonitor::endFrame() {
// #if PERFORMANCE_ENABLED
    if (!measuringFrame) return;

    // End total frame GPU measurement
    endGPUQuery(QueryLabel::FRAME_TOTAL);

    // End CPU frame timing
    auto frameEndTime = Clock::now();
    currentFrame.cpuFrameTime = std::chrono::duration_cast<Nanoseconds>(frameEndTime - frameStartTime);
    currentFrame.cpuDataValid = true;

    // Retrieve and store GPU results
    validateAndStoreFrame();

    measuringFrame = false;
// #endif
}

void PerformanceMonitor::beginGPUQuery(QueryLabel label) {
// #if PERFORMANCE_ENABLED
    if (!measuringFrame || !queriesInitialized) return;

    GLuint queryId = queryObjects[static_cast<size_t>(label)];

    // Use glQueryCounter for timestamp queries
    glQueryCounter(queryId, GL_TIMESTAMP);

    currentQuery = label;
// #endif
}

void PerformanceMonitor::endGPUQuery(QueryLabel label) {
// #if PERFORMANCE_ENABLED
    if (!measuringFrame || !queriesInitialized) return;

    GLuint queryId = queryObjects[static_cast<size_t>(label)];

    // For timer queries, we need to query the end timestamp
    glQueryCounter(queryId + 1, GL_TIMESTAMP); // Use next query ID for end timestamp

    // We'll calculate the difference when retrieving results
// #endif
}

void PerformanceMonitor::beginCPUSubmitMeasurement() {
// #if PERFORMANCE_ENABLED
    submitStartTime = Clock::now();
// #endif
}

void PerformanceMonitor::endCPUSubmitMeasurement() {
// #if PERFORMANCE_ENABLED
    auto submitEndTime = Clock::now();
    currentFrame.cpuSubmitTime = std::chrono::duration_cast<Nanoseconds>(submitEndTime - submitStartTime);
// #endif
}

uint64_t PerformanceMonitor::getQueryResult(GLuint queryId) const {
    GLuint64 result = 0;

    // Check if result is available (non-blocking)
    GLint available = 0;
    glGetQueryObjectiv(queryId, GL_QUERY_RESULT_AVAILABLE, &available);

    if (available) {
        glGetQueryObjectui64v(queryId, GL_QUERY_RESULT, &result);
    } else {
        // Result not available yet - wait for it (blocking)
        // In production, you might want to handle this differently
        glGetQueryObjectui64v(queryId, GL_QUERY_RESULT, &result);
    }

    return static_cast<uint64_t>(result);
}

void PerformanceMonitor::resetCurrentFrame() {
    currentFrame = FrameTiming{};
    currentFrame.frameNumber = frameCounter;
}

void PerformanceMonitor::validateAndStoreFrame() {
// #if PERFORMANCE_ENABLED
    if (!queriesInitialized) return;

    bool allValid = true;

    // Retrieve GPU timings
    // For each section, calculate elapsed time between start and end timestamps
    for (size_t i = 0; i < static_cast<size_t>(QueryLabel::COUNT); ++i) {
        GLuint startQuery = queryObjects[i];
        GLuint endQuery = queryObjects[i] + 1; // We reserved pairs of queries

        uint64_t startTime = getQueryResult(startQuery);
        uint64_t endTime = getQueryResult(endQuery);

        if (endTime > startTime) {
            uint64_t elapsed = endTime - startTime;

            switch (static_cast<QueryLabel>(i)) {
                case QueryLabel::FRAME_TOTAL:
                    currentFrame.gpuTotalTime = elapsed;
                    break;
                case QueryLabel::SECTION_1_2:
                    currentFrame.gpuSection12 = elapsed;
                    break;
                case QueryLabel::SECTION_1_4:
                    currentFrame.gpuSection14 = elapsed;
                    break;
                case QueryLabel::SECTION_1_5:
                    currentFrame.gpuSection15 = elapsed;
                    break;
                default:
                    break;
            }
        } else {
            allValid = false;
        }
    }

    currentFrame.gpuDataValid = allValid;

    // Store frame data
    frameTimings.push_back(currentFrame);

    // Keep only last N frames to avoid memory growth
    const size_t maxFrames = 1000;
    if (frameTimings.size() > maxFrames) {
        frameTimings.erase(frameTimings.begin(),
                          frameTimings.begin() + (frameTimings.size() - maxFrames));
    }
// #endif
}

void PerformanceMonitor::calculateStatistics() {
// #if PERFORMANCE_ENABLED
    statistics = Stats{};

    for (const auto& frame : frameTimings) {
        if (!frame.gpuDataValid || !frame.cpuDataValid) continue;

        statistics.validFrames++;

        // GPU statistics
        statistics.totalGPUTime += frame.gpuTotalTime;
        statistics.minGPUTime = std::min(statistics.minGPUTime, frame.gpuTotalTime);
        statistics.maxGPUTime = std::max(statistics.maxGPUTime, frame.gpuTotalTime);

        // CPU statistics
        statistics.totalCPUTime += frame.cpuSubmitTime;
        statistics.minCPUTime = std::min(statistics.minCPUTime, frame.cpuSubmitTime);
        statistics.maxCPUTime = std::max(statistics.maxCPUTime, frame.cpuSubmitTime);
    }

    if (statistics.validFrames > 0) {
        statistics.avgGPUTime = statistics.totalGPUTime / statistics.validFrames;
        statistics.avgCPUTime = statistics.totalCPUTime / statistics.validFrames;
    }

    statsCalculated = true;
// #endif
}

void PerformanceMonitor::printStatistics() const {
// #if PERFORMANCE_ENABLED
    if (!statsCalculated || statistics.validFrames == 0) {
        std::print("No valid performance data collected\n");
        return;
    }

    std::print("\n=== PERFORMANCE STATISTICS ===\n");
    std::print("Frames analyzed: {}\n", statistics.validFrames);

    std::print("\nGPU Timing (nanoseconds):\n");
    std::print("  Total: {:L}\n", statistics.totalGPUTime);
    std::print("  Average per frame: {:L}\n", statistics.avgGPUTime);
    std::print("  Min: {:L}\n", statistics.minGPUTime);
    std::print("  Max: {:L}\n", statistics.maxGPUTime);

    std::print("\nCPU Submit Timing (nanoseconds):\n");
    std::print("  Total: {:L}\n", statistics.totalCPUTime.count());
    std::print("  Average per frame: {:L}\n", statistics.avgCPUTime.count());
    std::print("  Min: {:L}\n", statistics.minCPUTime.count());
    std::print("  Max: {:L}\n", statistics.maxCPUTime.count());

    // Convert to milliseconds for readability
    double avgGPUMs = statistics.avgGPUTime / 1'000'000.0;
    double avgCPUMs = statistics.avgCPUTime.count() / 1'000'000.0;

    std::print("\nAverages (milliseconds):\n");
    std::print("  GPU: {:.3f} ms\n", avgGPUMs);
    std::print("  CPU Submit: {:.3f} ms\n", avgCPUMs);
    std::print("  Estimated FPS: {:.1f}\n", 1'000.0 / (avgGPUMs + avgCPUMs));
// #endif
}

void PerformanceMonitor::printCurrentFrame() const {
// #if PERFORMANCE_ENABLED
    std::print("\nFrame {}:\n", currentFrame.frameNumber);

    if (currentFrame.gpuDataValid) {
        std::print("  GPU Timings (ns):\n");
        std::print("    Total: {:L}\n", currentFrame.gpuTotalTime);
        std::print("    Section 1.2 (Terrain): {:L}\n", currentFrame.gpuSection12);
        std::print("    Section 1.4 (Landing Pads): {:L}\n", currentFrame.gpuSection14);
        std::print("    Section 1.5 (Vehicle): {:L}\n", currentFrame.gpuSection15);
    }

    if (currentFrame.cpuDataValid) {
        std::print("  CPU Timings (ns):\n");
        std::print("    Submit: {:L}\n", currentFrame.cpuSubmitTime.count());
        std::print("    Frame: {:L}\n", currentFrame.cpuFrameTime.count());
    }
// #endif
}

bool PerformanceMonitor::saveToCSV(const std::string& filename) const {
// #if PERFORMANCE_ENABLED
    FILE* file = fopen(filename.c_str(), "w");
    if (!file) return false;

    // Write CSV header
    fprintf(file, "Frame,GPU_Total_ns,GPU_Section1_2_ns,GPU_Section1_4_ns,GPU_Section1_5_ns,CPU_Submit_ns,CPU_Frame_ns\n");

    // Write data
    for (const auto& frame : frameTimings) {
        if (!frame.gpuDataValid || !frame.cpuDataValid) continue;

        fprintf(file, "%llu,%llu,%llu,%llu,%llu,%lld,%lld\n",
                static_cast<unsigned long long>(frame.frameNumber),
                static_cast<unsigned long long>(frame.gpuTotalTime),
                static_cast<unsigned long long>(frame.gpuSection12),
                static_cast<unsigned long long>(frame.gpuSection14),
                static_cast<unsigned long long>(frame.gpuSection15),
                static_cast<long long>(frame.cpuSubmitTime.count()),
                static_cast<long long>(frame.cpuFrameTime.count()));
    }

    fclose(file);
    return true;
// #else
//     return false;
// #endif
}

// Singleton instance
PerformanceMonitor& getMonitor() {
    static PerformanceMonitor instance;
    return instance;
}

} // namespace Performance