#!/bin/bash
# UMC-SDK Build Script for Linux/macOS

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default values
BUILD_TYPE="Release"
BUILD_DIR="build"
INSTALL_PREFIX="/usr/local"
ENABLE_LLVM="ON"
ENABLE_TREE_SITTER="ON"
ENABLE_ANTLR="ON"
VERBOSE="OFF"
RUN_TESTS="ON"
RUN_BENCHMARKS="OFF"

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to detect OS
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        echo "linux"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macos"
    else
        echo "unknown"
    fi
}

# Function to check dependencies
check_dependencies() {
    print_status "Checking dependencies..."
    
    # Check for CMake
    if ! command -v cmake &> /dev/null; then
        print_error "CMake is not installed"
        exit 1
    fi
    
    # Check for compiler
    if command -v gcc &> /dev/null; then
        print_status "Found GCC: $(gcc --version | head -n1)"
    elif command -v clang &> /dev/null; then
        print_status "Found Clang: $(clang --version | head -n1)"
    else
        print_error "No C++ compiler found"
        exit 1
    fi
    
    # Check for optional dependencies
    if command -v llvm-config &> /dev/null; then
        print_status "Found LLVM: $(llvm-config --version)"
        ENABLE_LLVM="ON"
    else
        print_warning "LLVM not found, building without LLVM support"
        ENABLE_LLVM="OFF"
    fi
    
    OS=$(detect_os)
    print_status "Detected OS: $OS"
}

# Function to install dependencies on Ubuntu/Debian
install_deps_ubuntu() {
    print_status "Installing dependencies on Ubuntu/Debian..."
    
    sudo apt-get update
    sudo apt-get install -y \
        cmake \
        build-essential \
        llvm-dev \
        libclang-dev \
        libtree-sitter-dev \
        libantlr4-runtime-dev \
        pkg-config
    
    print_success "Dependencies installed"
}

# Function to install dependencies on macOS
install_deps_macos() {
    print_status "Installing dependencies on macOS..."
    
    if ! command -v brew &> /dev/null; then
        print_error "Homebrew not found. Please install Homebrew first."
        exit 1
    fi
    
    brew install cmake llvm tree-sitter antlr4
    
    print_success "Dependencies installed"
}

# Function to configure build
configure_build() {
    print_status "Configuring build..."
    
    CMAKE_ARGS=(
        "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
        "-DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX"
        "-DENABLE_LLVM=$ENABLE_LLVM"
        "-DENABLE_TREE_SITTER=$ENABLE_TREE_SITTER"
        "-DENABLE_ANTLR=$ENABLE_ANTLR"
        "-DCMAKE_VERBOSE_MAKEFILE=$VERBOSE"
    )
    
    if [[ "$OS" == "macos" ]]; then
        CMAKE_ARGS+=("-DCMAKE_OSX_DEPLOYMENT_TARGET=10.15")
    fi
    
    cmake -B "$BUILD_DIR" "${CMAKE_ARGS[@]}"
    
    print_success "Build configured"
}

# Function to build project
build_project() {
    print_status "Building project..."
    
    if [[ "$VERBOSE" == "ON" ]]; then
        cmake --build "$BUILD_DIR" --verbose
    else
        # Use all available cores
        if command -v nproc &> /dev/null; then
            CORES=$(nproc)
        elif command -v sysctl &> /dev/null; then
            CORES=$(sysctl -n hw.ncpu)
        else
            CORES=4
        fi
        
        cmake --build "$BUILD_DIR" --parallel "$CORES"
    fi
    
    print_success "Build completed"
}

# Function to run tests
run_tests() {
    if [[ "$RUN_TESTS" == "ON" ]]; then
        print_status "Running tests..."
        
        cd "$BUILD_DIR"
        if ctest --output-on-failure; then
            print_success "All tests passed"
        else
            print_error "Some tests failed"
            return 1
        fi
        cd ..
    fi
}

# Function to run benchmarks
run_benchmarks() {
    if [[ "$RUN_BENCHMARKS" == "ON" ]]; then
        print_status "Running benchmarks..."
        
        cd "$BUILD_DIR"
        if ./ucc --benchmark; then
            print_success "Benchmarks completed"
        else
            print_warning "Some benchmarks failed"
        fi
        cd ..
    fi
}

# Function to install project
install_project() {
    print_status "Installing project..."
    
    cmake --install "$BUILD_DIR"
    
    print_success "Installation completed"
}

# Function to create package
create_package() {
    print_status "Creating package..."
    
    cd "$BUILD_DIR"
    cpack
    
    print_success "Package created"
    cd ..
}

# Function to clean build
clean_build() {
    print_status "Cleaning build directory..."
    
    rm -rf "$BUILD_DIR"
    
    print_success "Build cleaned"
}

# Function to show help
show_help() {
    cat << EOF
UMC-SDK Build Script

Usage: $0 [OPTIONS]

OPTIONS:
    -t, --type TYPE         Build type (Debug|Release|RelWithDebInfo) [default: Release]
    -d, --dir DIR           Build directory [default: build]
    -p, --prefix PREFIX     Install prefix [default: /usr/local]
    --llvm                  Enable LLVM support [default: ON]
    --no-llvm               Disable LLVM support
    --tree-sitter           Enable Tree-sitter support [default: ON]
    --no-tree-sitter        Disable Tree-sitter support
    --antlr                 Enable ANTLR support [default: ON]
    --no-antlr              Disable ANTLR support
    -v, --verbose           Verbose build
    --test                  Run tests [default: ON]
    --no-test               Skip tests
    --benchmark             Run benchmarks [default: OFF]
    --install               Install after build
    --package               Create package
    --clean                 Clean build directory
    --install-deps          Install dependencies
    -h, --help              Show this help message

EXAMPLES:
    $0                      # Default build
    $0 -t Debug -v          # Debug build with verbose output
    $0 --no-llvm --install  # Build without LLVM and install
    $0 --install-deps       # Install dependencies and build

EOF
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -t|--type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -d|--dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        -p|--prefix)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        --llvm)
            ENABLE_LLVM="ON"
            shift
            ;;
        --no-llvm)
            ENABLE_LLVM="OFF"
            shift
            ;;
        --tree-sitter)
            ENABLE_TREE_SITTER="ON"
            shift
            ;;
        --no-tree-sitter)
            ENABLE_TREE_SITTER="OFF"
            shift
            ;;
        --antlr)
            ENABLE_ANTLR="ON"
            shift
            ;;
        --no-antlr)
            ENABLE_ANTLR="OFF"
            shift
            ;;
        -v|--verbose)
            VERBOSE="ON"
            shift
            ;;
        --test)
            RUN_TESTS="ON"
            shift
            ;;
        --no-test)
            RUN_TESTS="OFF"
            shift
            ;;
        --benchmark)
            RUN_BENCHMARKS="ON"
            shift
            ;;
        --install)
            INSTALL_AFTER_BUILD="ON"
            shift
            ;;
        --package)
            CREATE_PACKAGE="ON"
            shift
            ;;
        --clean)
            clean_build
            exit 0
            ;;
        --install-deps)
            INSTALL_DEPS="ON"
            shift
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

# Main execution
main() {
    print_status "UMC-SDK Build Script"
    print_status "===================="
    
    # Install dependencies if requested
    if [[ "$INSTALL_DEPS" == "ON" ]]; then
        OS=$(detect_os)
        case $OS in
            linux)
                install_deps_ubuntu
                ;;
            macos)
                install_deps_macos
                ;;
            *)
                print_error "Unsupported OS for automatic dependency installation"
                exit 1
                ;;
        esac
    fi
    
    # Check dependencies
    check_dependencies
    
    # Clean if requested
    if [[ "$CLEAN" == "ON" ]]; then
        clean_build
    fi
    
    # Configure build
    configure_build
    
    # Build project
    build_project
    
    # Run tests
    run_tests
    
    # Run benchmarks
    run_benchmarks
    
    # Install if requested
    if [[ "$INSTALL_AFTER_BUILD" == "ON" ]]; then
        install_project
    fi
    
    # Create package if requested
    if [[ "$CREATE_PACKAGE" == "ON" ]]; then
        create_package
    fi
    
    print_success "Build script completed successfully!"
    print_status "Executable location: $BUILD_DIR/Debug/ucc (or Release/ucc)"
}

# Run main function
main
