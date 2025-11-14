#!/bin/bash

# Load necessary modules
module load gcc

# Configuration
PROJECT_DIR="$HOME/github-projects/uni/graphics/cw2"

# Get core count for parallel build
CORES=$(grep -c ^processor /proc/cpuinfo 2>/dev/null || echo 4)
echo "  > Detected $CORES CPU cores for parallel build."

# Navigate to project directory
echo "  > Entering cw1 project directory..."
cd "$PROJECT_DIR" || exit 1

# Convert input to lowercase for case-insensitive matching
CMD="${1,,}"

echo "  > You are running command: <$CMD>"

# Build configuration
case "$CMD" in
    "bench"|"benchmark")
        CONFIG="release_x64"
        FILE_CONFIG="release-x64"
        BUILD_TYPE="Release"
        ;;
    *)
        CONFIG="debug_x64"
        FILE_CONFIG="debug-x64"
        BUILD_TYPE="Debug"
        ;;
esac

# Build process
echo "  > Generating make files..."
./premake5 gmake

echo "  > Building project ($BUILD_TYPE) using $CORES cores..."
make -j"$CORES" config="$CONFIG"

# Execution
case "$1" in
    ""|"main")
        ./bin/main-$FILE_CONFIG-gcc.exe
        ;;
    "li"|"lines")
        ./bin/lines-sandbox-$FILE_CONFIG-gcc.exe
        ./bin/lines-test-$FILE_CONFIG-gcc.exe -d yes -v high # -s
        ;;
    "tri"|"triangles")
        ./bin/triangles-sandbox-$FILE_CONFIG-gcc.exe
        ./bin/triangles-test-$FILE_CONFIG-gcc.exe -d yes -v high # -s
        ;;
    "blit")
        ./bin/blit-benchmark-$FILE_CONFIG-gcc.exe
        ;;
    "sand"|"sandbox")
        ./bin/lines-sandbox-$FILE_CONFIG-gcc.exe
        ./bin/triangles-sandbox-$FILE_CONFIG-gcc.exe
        ;;
    "test")
        ./bin/lines-test-$FILE_CONFIG-gcc.exe -d yes -v high # -s
        ./bin/triangles-test-$FILE_CONFIG-gcc.exe -d yes -v high # -s
        ;;
    "bench"|"benchmark")
        echo "Running benchmarks..."
        if [ $CORES -eq 8 ]; then
            echo "  > You're on the Lenovo!"
            echo "  > Setting CPU governor to performance mode..."
            sudo cpupower frequency-set -g performance

            # ./bin/lines-benchmark-$FILE_CONFIG-gcc.exe \
            # --benchmark_format=console \
            # --benchmark_out=lines_i_benchmark_results_nano.json

            ./bin/blit-benchmark-$FILE_CONFIG-gcc.exe \
            --benchmark_format=console \
            --benchmark_out=blit_benchmark_results.json

            echo "  > Setting CPU governor to powersave mode..."
            sudo cpupower frequency-set -g powersave
        else
            echo " Done "
        fi
        ;;
    "deb"|"debug")
        ./bin/main-debug_x64-gcc.exe
        ./bin/lines-sandbox-debug-x64-gcc.exe
        ./bin/lines-test-debug-x64-gcc.exe -d yes -v high # -s
        ./bin/triangles-sandbox-debug-x64-gcc.exe
        ./bin/triangles-test-debug-x64-gcc.exe -d yes -v high # -s
        ;;
    "rel"|"release")
        ./bin/main-release_x64-gcc.exe
        ./bin/lines-sandbox-release-x64-gcc.exe
        ./bin/lines-test-release-x64-gcc.exe -d yes -v high # -s
        ./bin/triangles-sandbox-release-x64-gcc.exe
        ./bin/triangles-test-release-x64-gcc.exe -d yes -v high # -s
        if [ $CORES -eq 8 ]; then
            echo "  > You're on the Lenovo!"
            echo "  > Setting CPU governor to performance mode..."
            sudo cpupower frequency-set -g performance
            ./bin/lines-benchmark-release-x64-gcc.exe
            ./bin/blit-benchmark-release-x64-gcc.exe
            echo "  > Setting CPU governor to powersave mode..."
            sudo cpupower frequency-set -g powersave
        else
            echo " Done "
        fi
        ;;
    "m"|"make")
        echo "Build completed."
        ;;
    "clean")
        rm -rf _build_ bin lib
        du -sh ./
        ;;
    *)
        echo "Usage: $0 [command]"
        echo "Commands:"
        echo "  (no command) - Build and execute main executable"
        echo "  li           - Build and execute all line-related executables"
        echo "  tri          - Build and execute all triangle-related executables"
        echo "  blit         - Build and execute all blit-related executables"
        echo "  sand         - Build and execute all sandbox-related executables"
        echo "  test         - Build and execute all test-related executables"
        echo "  bench        - Build and execute all benchmark-related executables"
        echo "  debug        - Build and execute all debug executables"
        echo "  release      - Build and execute all release executables"
        echo "  make         - Build the project without running executables"
        echo "  clean        - Clean all build artifacts to free disk space"
        exit 1
        ;;
esac