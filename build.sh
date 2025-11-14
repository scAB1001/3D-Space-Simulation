#!/bin/bash

# Load necessary modules
module load gcc

# Configuration
PROJECT_DIR="./cw2/"

# Get core count for parallel build
CORES=$(grep -c ^processor /proc/cpuinfo 2>/dev/null || echo 4)
echo "  > Detected $CORES CPU cores for parallel build."

# Navigate to project directory
echo "  > Entering cw2 project directory..."
cd "$PROJECT_DIR" || exit 1

# Parse command line arguments
TARGET="${1,,}"  # main or vmlibtest
CONFIG="${2,,}"   # debug or release

# Validate arguments
if [[ -z "$TARGET" ]]; then
    TARGET="main"
fi

if [[ -z "$CONFIG" ]]; then
    CONFIG="debug"  # default to debug
fi

# Map configurations to premake formats
case "$CONFIG" in
    "deb"|"debug")
        PREMAKE_CONFIG="debug_x64"
        FILE_CONFIG="debug-x64"
        ;;
    "rel"|"release")
        PREMAKE_CONFIG="release_x64"
        FILE_CONFIG="release-x64"
        ;;
    *)
        echo "Error: Invalid configuration '$CONFIG'. Use 'debug' or 'release'"
        exit 1
        ;;
esac

# Map targets to executable names
case "$TARGET" in
    "m"|"main")
        EXECUTABLE="main-$FILE_CONFIG-gcc.exe"
        ;;
    "vm"|"vmlibtest")
        EXECUTABLE="vmlib-test-$FILE_CONFIG-gcc.exe"
        ;;
    "mk"|"make")
        EXECUTABLE="x0x0x0x0x0x"
        ;;
    *)
        echo "Error: Invalid target '$TARGET'. Use 'main', 'vmlibtest', or 'make'"
        echo "Usage: $0 [main|vmlibtest|make] [debug|release]"
        exit 1
        ;;
esac

EXECUTABLE_PATH="./bin/$EXECUTABLE"

echo "  > Building target: $TARGET"
echo "  > Configuration: $CONFIG"
echo "  > Executable: $EXECUTABLE"

# Build process
echo "  > Generating make files..."
./premake5 gmake

echo "  > Building project ($PREMAKE_CONFIG) using $CORES cores..."
make -j"$CORES" config="$PREMAKE_CONFIG"

# Check if build was successful
if [[ $? -ne 0 ]]; then
    echo "  > Build failed!"
    exit 1
fi

# For "make" target, just exit after successful build
if [[ "$TARGET" == "mk" || "$TARGET" == "make" ]]; then
    echo "  > Build completed successfully."
    exit 0
fi

# Check if executable exists
if [[ ! -f "$EXECUTABLE_PATH" ]]; then
    echo "  > Error: Executable not found: <$EXECUTABLE_PATH>"
    echo "  > Available executables in bin/:"
    ls -ll ./bin/ 2>/dev/null || echo "    (bin directory not found)"
    exit 1
fi

echo "  > Build successful! Running $EXECUTABLE..."

# CPU governor setup for benchmarking (only on Lenovo with 8 cores AND release builds)
if [[ $CORES -eq 8 && "$CONFIG" == "release" ]]; then
    echo "  > You're on the Lenovo! Setting CPU governor to performance mode..."
    sudo cpupower frequency-set -g performance
fi

# Execute the built executable
"$EXECUTABLE_PATH"

# Restore CPU governor if changed (only if we set it to performance)
if [[ $CORES -eq 8 && "$CONFIG" == "release" ]]; then
    echo "  > Restoring CPU governor to powersave mode..."
    sudo cpupower frequency-set -g powersave
fi