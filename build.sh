#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

print_header() {
    echo ""
    echo -e "${CYAN}========================================${NC}"
    echo -e "${CYAN}  $1${NC}"
    echo -e "${CYAN}========================================${NC}"
}

print_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

print_info() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

print_indent() {
    echo -e "    $1"
}


load_modules() {
    print_info "Loading required modules..."
    module load gcc
}

get_core_count() {
    # This function sets CORES as a global variable
    CORES=$(grep -c ^processor /proc/cpuinfo 2>/dev/null || echo 4)
    print_info "Detected ${YELLOW}$CORES${BLUE} CPU cores for parallel build."
}

configure_dir() {
    # PROJECT_DIR="$HOME/github-projects/uni/comp3811-labs"
    # PROJECT_DIR="./comp3811-cw2"
    # CW_DIR="cw2"
    PROJECT_DIR="./cw2/"

    # Navigate to project directory
    print_info "Entering $PROJECT_DIR project directory..."
    if [ ! -d "$PROJECT_DIR" ]; then
        print_error "Directory $PROJECT_DIR does not exist!"
        exit 1
    fi
    cd "$PROJECT_DIR" || exit 1
}

get_target_config() {
    local target="$1"
    local mode="$2"

    print_info "Target: <${YELLOW}${target:-main}${BLUE}>, Mode: <${YELLOW}${mode:-debug}${BLUE}>"

    # Map targets to executable names
    case "${target,,}" in
        "m"|"main")
            EXECUTABLE_NAME="main"
            ;;
        "vm"|"vmlibtest")
            EXECUTABLE_NAME="vmlib-test"
            ;;
        "mk"|"make")
            EXECUTABLE_NAME="make"
            ;;
        "val")
            EXECUTABLE_NAME="valgrind"
            ;;
        *)
            print_error "Invalid target '$target'. Use 'main', 'vmlibtest', 'make', or 'valgrind'"
            echo "Usage: $0 [main|vmlibtest|make|valgrind] [debug|release]"
            exit 1
            ;;
    esac

    # Build configuration
    case "${mode,,}" in
        "rel"|"release"|"bench"|"benchmark")
            CONFIG="release_x64"
            FILE_CONFIG="release-x64"
            BUILD_TYPE="Release"
            ;;
        "deb"|"debug"|"")
            CONFIG="debug_x64"
            FILE_CONFIG="debug-x64"
            BUILD_TYPE="Debug"
            ;;
        *)
            print_warning "Unknown mode '$mode'. Using debug mode."
            CONFIG="debug_x64"
            FILE_CONFIG="debug-x64"
            BUILD_TYPE="Debug"
            ;;
    esac

    # Set executable path (except for make target)
    if [[ "$EXECUTABLE_NAME" != "make" ]]; then
        EXECUTABLE_PATH="./bin/${EXECUTABLE_NAME}-${FILE_CONFIG}-gcc.exe"
    fi
}

setup_environment() {
    print_header "Setting up Environment"
    get_core_count
    configure_dir
    get_target_config "$1" "$2"
    print_success "Environment setup complete."
}

build() {
    print_header "Building cw2 Project"
    print_info "Generating make files..."

    if ! ./premake5 gmake; then
        print_error "Failed to generate make files!"
        exit 1
    fi

    print_info "Building project ($BUILD_TYPE) using $CORES cores..."
    if ! make -j"$CORES" config="$CONFIG"; then
        print_error "Build failed!"
        exit 1
    else
        print_success "Build completed successfully."
    fi
}

run() {
    # For "make" target, just exit after successful build
    if [[ "$EXECUTABLE_NAME" == "make" ]]; then
        print_success "Build-only mode completed."
        exit 0
    fi

    if [[ "$EXECUTABLE_NAME" == "valgrind" ]]; then
        EXECUTABLE_PATH="./bin/main-${FILE_CONFIG}-gcc.exe"
        print_header "Running Valgrind on Main Executable"
        if [ ! -f "$EXECUTABLE_PATH" ]; then
            print_error "Main executable not found for Valgrind: $EXECUTABLE_PATH"
            exit 1
        fi
        print_info "Running Valgrind: valgrind --leak-check=full $EXECUTABLE_PATH"
        valgrind --leak-check=full --show-leak-kinds=all "$EXECUTABLE_PATH"
        exit 0
    fi

    print_header "Running $EXECUTABLE_NAME Executable"

    if [ ! -f "$EXECUTABLE_PATH" ]; then
        print_error "Executable not found: $EXECUTABLE_PATH"
        print_info "Available executables in bin/:"
        ls -la ./bin/ 2>/dev/null || echo "    (bin directory not found)"
        exit 1
    fi

    # CPU governor setup for benchmarking (only on Lenovo with 8 cores AND release builds)
    if [[ $CORES -eq 8 && "$FILE_CONFIG" == "release-x64" ]]; then
        print_info "You're on the Lenovo! Setting CPU governor to performance mode..."
        sudo cpupower frequency-set -g performance
    fi

    print_info "Running: $EXECUTABLE_PATH"
    "$EXECUTABLE_PATH"

    # Restore CPU governor if changed
    if [[ $CORES -eq 8 && "$FILE_CONFIG" == "release-x64" ]]; then
        print_info "Restoring CPU governor to powersave mode..."
        sudo cpupower frequency-set -g powersave
    fi
}

# Show usage if no arguments provided
show_usage() {
    echo -e "${RED}-----------------------------------------------------------${NC}"
    echo -e "  ${CYAN}Usage: $0 <target> [mode]${NC}"
    echo ""
    echo -e " ${BLUE}Targets:${NC}"
    echo "   main       - Build and run main executable (default)"
    echo "   vmlibtest  - Build and run vmlib-test executable"
    echo "   make       - Build only, don't run"
    echo ""
    echo -e " ${BLUE}Modes:${NC}"
    echo "   debug      - Debug build (default)"
    echo "   release    - Release build"
    echo ""
    echo -e " ${BLUE}Examples:${NC}"
    echo "   $0 main debug"
    echo "   $0 vmlibtest release"
    echo "   $0 main"
    echo "   $0 make"
    echo -e "${RED}-----------------------------------------------------------${NC}"
}

# Main script execution
main() {
    # If only ./build.sh is run, show usage
    if [[ $# -eq 0 ]]; then
        # setup_environment "main" "debug"
        show_usage
        exit 1
    else
        setup_environment "$1" "$2"
    fi

    build
    run
}

# Run main function with all arguments
main "$@"

# LEAK SUMMARY from first Valgrind run:
# ==130206==    definitely lost: 616 bytes in 2 blocks
# ==130206==    indirectly lost: 1,769 bytes in 2 blocks
# ==130206==      possibly lost: 0 bytes in 0 blocks
# ==130206==    still reachable: 80,439 bytes in 920 blocks
# ==130206==         suppressed: 0 bytes in 0 blocks
# ==130206== Reachable blocks (those to which a pointer was found) are not shown.
# ==130206== To see them, rerun with: --leak-check=full --show-leak-kinds=all
