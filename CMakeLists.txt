cmake_minimum_required(VERSION 3.16)
project(TouchdownOS VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_TESTS "Build unit tests" OFF)
option(ENABLE_DEBUG "Enable debug symbols and logging" ON)

# Build configuration
if(ENABLE_DEBUG)
    add_compile_options(-g -O0 -Wall -Wextra)
    add_definitions(-DDEBUG_BUILD)
else()
    add_compile_options(-O3 -Wall)
    add_definitions(-DNDEBUG)
endif()

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(Python3 COMPONENTS Interpreter Development)

pkg_check_modules(DRM REQUIRED libdrm)
pkg_check_modules(DBUS REQUIRED dbus-1)
pkg_check_modules(SYSTEMD REQUIRED libsystemd)

# LVGL configuration
set(LVGL_DIR ${CMAKE_SOURCE_DIR}/third_party/lvgl)
if(NOT EXISTS ${LVGL_DIR})
    message(WARNING "LVGL not found in third_party/lvgl. Please run: git submodule update --init")
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/third_party/nlohmann
    ${DRM_INCLUDE_DIRS}
    ${DBUS_INCLUDE_DIRS}
    ${SYSTEMD_INCLUDE_DIRS}
    ${LVGL_DIR}
)

# Add subdirectories
add_subdirectory(third_party)
add_subdirectory(src)
add_subdirectory(apps)

# Installation paths
set(CMAKE_INSTALL_PREFIX /usr)
set(SYSTEMD_UNIT_DIR /etc/systemd/system)
set(TOUCHDOWN_CONFIG_DIR /etc/touchdown)
set(TOUCHDOWN_APP_DIR /usr/share/touchdown/apps)

# Install configuration files
install(DIRECTORY config/systemd/ DESTINATION ${SYSTEMD_UNIT_DIR})
install(DIRECTORY config/touchdown/ DESTINATION ${TOUCHDOWN_CONFIG_DIR})

# Package configuration
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "touchdownos")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "TouchdownOS - Custom LVGL Wearable Linux OS")
set(CPACK_PACKAGE_CONTACT "TouchdownOS Project")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libdrm2, libdbus-1-3, libsystemd0, python3")

include(CPack)
