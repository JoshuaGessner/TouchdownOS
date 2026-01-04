# Building TouchdownOS

This guide covers building TouchdownOS for both native (x86_64) development and cross-compilation for Raspberry Pi Zero 2 W.

## Build System

TouchdownOS uses CMake with the following targets:
- **Native build**: For development/testing on x86_64 Linux
- **Cross-compile**: For deployment on Raspberry Pi Zero 2 W (ARM)

## Prerequisites

### Common Dependencies

```bash
sudo apt-get update
sudo apt-get install \
    build-essential \
    cmake \
    pkg-config \
    git \
    libdrm-dev \
    libdbus-1-dev \
    libsystemd-dev \
    libudev-dev \
    libinput-dev \
    python3-dev \
    python3-pip \
    pybind11-dev
```

### Cross-Compilation Toolchain

```bash
# ARM GCC toolchain
sudo apt-get install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf

# ARM target libraries (optional, for linking against ARM libs)
sudo dpkg --add-architecture armhf
sudo apt-get update
sudo apt-get install \
    libdrm-dev:armhf \
    libdbus-1-dev:armhf \
    libsystemd-dev:armhf
```

## Building

### 1. Clone and Initialize

```bash
git clone https://github.com/JoshuaGessner/TouchdownOS.git
cd TouchdownOS
git submodule update --init --recursive
```

### 2. Native Build (x86_64)

For development and testing on your Linux machine:

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

This builds:
- `touchdown-shell` - Main shell executable
- `touchdown-power-service` - Power management service
- `touchdown-input-service` - Input handling service
- `touchdown_app.a` - App framework library
- `touchdown.so` - Python module (if pybind11 found)

### 3. Cross-Compilation (ARM)

For deployment on Raspberry Pi Zero 2 W:

```bash
mkdir build-arm
cd build-arm
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-pi-toolchain.cmake ..
make -j$(nproc)
```

**Troubleshooting:**

If you see "arm-linux-gnueabihf-gcc not found":
```bash
# Check if installed
which arm-linux-gnueabihf-gcc

# If not found, install
sudo apt-get install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf
```

If you see pkg-config errors for ARM libraries:
```bash
# Either install ARM libraries (recommended)
sudo dpkg --add-architecture armhf
sudo apt-get install libdrm-dev:armhf libdbus-1-dev:armhf

# Or disable pkg-config for cross-compile (not recommended)
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-pi-toolchain.cmake \
      -DPKG_CONFIG_EXECUTABLE=/bin/false ..
```

### 4. Create Debian Package

```bash
cd build-arm
cpack
```

This creates `touchdownos_0.1.0_armhf.deb` containing:
- Binaries in `/usr/bin/`
- Libraries in `/usr/lib/`
- System services in `/etc/systemd/system/`
- Configuration in `/etc/touchdown/`
- App manifests in `/usr/share/touchdown/apps/`

## Build Options

### CMake Options

```bash
# Enable debug symbols and verbose logging
cmake -DENABLE_DEBUG=ON ..

# Build tests (future)
cmake -DBUILD_TESTS=ON ..

# Custom install prefix
cmake -DCMAKE_INSTALL_PREFIX=/opt/touchdown ..
```

### Optimization Levels

**Development (default with ENABLE_DEBUG=ON):**
- `-g` - Debug symbols
- `-O0` - No optimization
- Verbose logging enabled

**Release (ENABLE_DEBUG=OFF):**
- `-O3` - Maximum optimization
- No debug symbols
- Minimal logging

## Testing Build

### Native Build

```bash
cd build
./src/shell/touchdown-shell
```

You'll need a DRM-capable display. For testing without hardware:
- Use QEMU with virtio-gpu
- Or modify display driver to use SDL2 (not implemented)

### Cross-Compiled Build

Copy to Raspberry Pi and test:

```bash
# Copy binary
scp touchdown-shell pi@raspberrypi:/tmp/

# SSH to Pi
ssh pi@raspberrypi

# Run (requires root for DRM/GPIO access)
sudo /tmp/touchdown-shell
```

## Installing on Development Machine

```bash
cd build
sudo make install
```

Files installed to:
- `/usr/bin/touchdown-shell`
- `/usr/bin/touchdown-*-service`
- `/etc/systemd/system/touchdown-*.service`
- `/etc/touchdown/shell.conf`

## Installing on Raspberry Pi

### Method 1: Debian Package (Recommended)

```bash
# On build machine
cd build-arm
cpack

# Copy to Pi
scp touchdownos_0.1.0_armhf.deb pi@raspberrypi:~

# On Pi
ssh pi@raspberrypi
sudo dpkg -i touchdownos_0.1.0_armhf.deb
sudo systemctl daemon-reload
sudo systemctl enable touchdown-shell
sudo reboot
```

### Method 2: Manual Install

```bash
# On build machine
cd build-arm
make install DESTDIR=./staging
tar -czf touchdown-install.tar.gz -C staging .

# Copy to Pi
scp touchdown-install.tar.gz pi@raspberrypi:~

# On Pi
ssh pi@raspberrypi
sudo tar -xzf touchdown-install.tar.gz -C /
sudo systemctl daemon-reload
sudo systemctl enable touchdown-shell
```

## Build Artifacts

After successful build:

```
build-arm/
├── src/
│   ├── shell/touchdown-shell           # Main executable
│   ├── services/touchdown-*-service    # Service daemons
│   ├── app/libtouchdown_app.a         # App framework
│   └── bindings/touchdown.so          # Python module
├── touchdownos_0.1.0_armhf.deb        # Debian package
└── install_manifest.txt               # Install file list
```

## Continuous Integration

Example GitHub Actions workflow:

```yaml
name: Build
on: [push, pull_request]
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
        with:
          submodules: recursive
      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf
          sudo apt-get install libdrm-dev libdbus-1-dev libsystemd-dev
      - name: Build
        run: |
          mkdir build-arm && cd build-arm
          cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-pi-toolchain.cmake ..
          make -j$(nproc)
          cpack
      - name: Upload artifact
        uses: actions/upload-artifact@v3
        with:
          name: touchdown-deb
          path: build-arm/*.deb
```

## Clean Build

```bash
# Remove build directory
rm -rf build build-arm

# Or clean only
cd build-arm
make clean
```

## Development Workflow

1. **Make changes** to source files
2. **Build natively** for quick iteration:
   ```bash
   cd build
   make -j$(nproc)
   ```
3. **Test on target** when ready:
   ```bash
   cd ../build-arm
   make -j$(nproc)
   scp src/shell/touchdown-shell pi@raspberrypi:/tmp/
   ssh pi@raspberrypi "sudo /tmp/touchdown-shell"
   ```

## Troubleshooting

### "Cannot find lvgl"

```bash
# Initialize submodules
git submodule update --init --recursive
```

### "pybind11 not found"

```bash
# Install pybind11
sudo apt-get install pybind11-dev
# Or with pip
pip3 install pybind11
```

### Cross-compile linker errors

The toolchain file looks for libraries in `/usr/arm-linux-gnueabihf/`. If you installed ARM libraries:

```bash
# Verify ARM libraries exist
ls /usr/arm-linux-gnueabihf/lib/
```

### Permission denied running shell

TouchdownOS needs root for DRM/GPIO access:

```bash
sudo ./touchdown-shell
```

Or add user to required groups:

```bash
sudo usermod -aG video,input,gpio $USER
```
