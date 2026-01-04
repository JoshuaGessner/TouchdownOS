#!/bin/bash
# Build script for TouchdownOS

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}TouchdownOS Build Script${NC}"
echo "================================"

# Configuration
BUILD_TYPE="${1:-Release}"
CROSS_COMPILE="${2:-ON}"
JOBS=$(nproc)

echo "Build Type: $BUILD_TYPE"
echo "Cross Compile: $CROSS_COMPILE"
echo "Parallel Jobs: $JOBS"
echo

# Check for required tools
command -v cmake >/dev/null 2>&1 || { echo -e "${RED}Error: cmake not found${NC}"; exit 1; }

if [ "$CROSS_COMPILE" == "ON" ]; then
    command -v arm-linux-gnueabihf-gcc >/dev/null 2>&1 || { 
        echo -e "${RED}Error: ARM cross-compiler not found${NC}"
        echo "Install with: sudo apt-get install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf"
        exit 1
    }
fi

# Initialize submodules
echo -e "${YELLOW}Initializing submodules...${NC}"
git submodule update --init --recursive

# Create build directory
BUILD_DIR="build-arm"
if [ "$CROSS_COMPILE" == "OFF" ]; then
    BUILD_DIR="build"
fi

echo -e "${YELLOW}Creating build directory: $BUILD_DIR${NC}"
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Configure
echo -e "${YELLOW}Configuring build...${NC}"
cmake .. \
    -DCROSS_COMPILE=$CROSS_COMPILE \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DENABLE_DEBUG=$([ "$BUILD_TYPE" == "Debug" ] && echo "ON" || echo "OFF")

# Build
echo -e "${YELLOW}Building TouchdownOS...${NC}"
make -j$JOBS

# Create package
echo -e "${YELLOW}Creating Debian package...${NC}"
cpack

echo
echo -e "${GREEN}Build completed successfully!${NC}"
echo
echo "Artifacts:"
ls -lh *.deb 2>/dev/null || echo "  No packages created"
echo
echo "Install with:"
echo "  scp touchdownos_*.deb pi@raspberrypi:~"
echo "  ssh pi@raspberrypi 'sudo dpkg -i touchdownos_*.deb'"
