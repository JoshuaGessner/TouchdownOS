# CMake Toolchain file for cross-compiling to Raspberry Pi Zero 2 W
# Usage: cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-pi-toolchain.cmake ..

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Specify the cross compiler
# If installed via package manager, these should work:
find_program(CMAKE_C_COMPILER NAMES arm-linux-gnueabihf-gcc)
find_program(CMAKE_CXX_COMPILER NAMES arm-linux-gnueabihf-g++)

if(NOT CMAKE_C_COMPILER)
    message(FATAL_ERROR "arm-linux-gnueabihf-gcc not found. Install with: sudo apt-get install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf")
endif()

if(NOT CMAKE_CXX_COMPILER)
    message(FATAL_ERROR "arm-linux-gnueabihf-g++ not found. Install with: sudo apt-get install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf")
endif()

# Where is the target environment located
set(CMAKE_FIND_ROOT_PATH /usr/arm-linux-gnueabihf)

# Adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Compiler flags for Pi Zero 2 W (ARM Cortex-A53)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-a+crc -mtune=cortex-a53 -mfpu=neon-fp-armv8 -mfloat-abi=hard")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8-a+crc -mtune=cortex-a53 -mfpu=neon-fp-armv8 -mfloat-abi=hard")
