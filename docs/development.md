# TouchdownOS Development Guide

## Building TouchdownOS

### Prerequisites

#### On Debian/Ubuntu Development Machine

```bash
# Install cross-compilation toolchain
sudo apt-get update
sudo apt-get install -y \
    build-essential cmake pkg-config git \
    gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf \
    libdrm-dev libdbus-1-dev libsystemd-dev

# Clone repository
git clone https://github.com/yourorg/touchdownos.git
cd touchdownos

# Initialize submodules
git submodule update --init --recursive
```

### Build Steps

#### Native Build (for testing on x86_64)

```bash
mkdir build && cd build
cmake -DENABLE_DEBUG=ON ..
make -j$(nproc)
```

#### Cross-Compilation for Raspberry Pi Zero 2 W

```bash
mkdir build-arm && cd build-arm
cmake -DCROSS_COMPILE=ON -DENABLE_DEBUG=OFF ..
make -j$(nproc)

# Create Debian package
cpack
```

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `CROSS_COMPILE` | OFF | Enable cross-compilation for ARM |
| `ENABLE_DEBUG` | ON | Include debug symbols and logging |
| `BUILD_TESTS` | OFF | Build unit tests |

## Installing on Raspberry Pi

### Method 1: Debian Package (Recommended)

```bash
# Transfer package to Pi
scp touchdownos_0.1.0_armhf.deb pi@raspberrypi:~

# Install on Pi
ssh pi@raspberrypi
sudo dpkg -i touchdownos_0.1.0_armhf.deb

# Install dependencies if needed
sudo apt-get install -f
```

### Method 2: Manual Installation

```bash
# Copy binaries
sudo cp build-arm/src/shell/touchdown-shell /usr/bin/
sudo cp build-arm/src/services/touchdown-*-service /usr/bin/

# Copy configuration
sudo mkdir -p /etc/touchdown
sudo cp -r config/touchdown/* /etc/touchdown/

# Copy systemd services
sudo cp config/systemd/*.service /etc/systemd/system/

# Reload systemd
sudo systemctl daemon-reload
```

## Raspberry Pi Setup

### 1. Base OS Installation

```bash
# Flash Raspberry Pi OS Lite (64-bit) to SD card
# Use Raspberry Pi Imager or:
wget https://downloads.raspberrypi.org/raspios_lite_arm64/images/.../raspios_lite.img.xz
xzcat raspios_lite.img.xz | sudo dd of=/dev/sdX bs=4M status=progress
sync
```

### 2. Enable Hardware Interfaces

Edit `/boot/config.txt`:

```ini
# Enable SPI for display
dtparam=spi=on

# Enable I2C for touch controller
dtparam=i2c_arm=on

# Load TouchdownOS device tree overlay
dtoverlay=touchdown-overlay

# Disable HDMI (headless)
dtoverlay=vc4-kms-v3d,nohdmi

# GPU memory (can be minimal for DRM/KMS)
gpu_mem=64
```

### 3. Install Device Tree Overlay

```bash
# Compile device tree overlay
sudo dtc -@ -I dts -O dtb -o /boot/overlays/touchdown-overlay.dtbo \
    config/dts/touchdown-overlay.dts

# Or copy pre-compiled
sudo cp build-arm/touchdown-overlay.dtbo /boot/overlays/
```

### 4. Configure Kernel Modules

Create `/etc/modules-load.d/touchdown.conf`:

```
# DRM drivers
drm
drm_kms_helper
drm_mipi_dbi

# I2C
i2c-dev
i2c-bcm2835

# GPIO
gpio_keys
```

### 5. Enable TouchdownOS Services

```bash
# Enable services
sudo systemctl enable touchdown-power.service
sudo systemctl enable touchdown-input.service
sudo systemctl enable touchdown-shell.service

# Start services
sudo systemctl start touchdown-shell.service

# Check status
systemctl status touchdown-shell.service
```

## Development Workflow

### Running on Desktop (Native Build)

TouchdownOS can run on a desktop Linux system with a simulated display for development:

```bash
# Install SDL2 for LVGL display simulation
sudo apt-get install libsdl2-dev

# Build with simulation support (TODO: add CMake option)
# For now, modify lv_conf.h to enable SDL display

# Run
./build/src/shell/touchdown-shell
```

### Debugging on Target

```bash
# View logs
journalctl -u touchdown-shell.service -f

# Enable debug logging
# Edit /etc/touchdown/shell.conf
# Set log_level=debug

# Restart service
sudo systemctl restart touchdown-shell.service

# Remote debugging with gdbserver
sudo apt-get install gdbserver
gdbserver :1234 /usr/bin/touchdown-shell
```

### Hot Reload Development

```bash
# On development machine: watch and rebuild
while inotifywait -r src/ include/; do
    cmake --build build-arm
    scp build-arm/src/shell/touchdown-shell pi@raspberrypi:/tmp/
done

# On Pi: stop service, run manually
sudo systemctl stop touchdown-shell.service
/tmp/touchdown-shell
```

## Code Structure

### Adding a New Driver

1. Create header in `include/touchdown/drivers/`
2. Implement in `src/drivers/`
3. Add to `src/drivers/CMakeLists.txt`
4. Register with shell in `src/shell/shell.cpp`

### Adding a System Service

1. Create service class extending `DBusInterface`
2. Implement D-Bus methods and signals
3. Create service executable with systemd integration
4. Add systemd service file
5. Register D-Bus interface in `/etc/dbus-1/system.d/`

### Creating a UI Screen

1. Create class in `include/touchdown/shell/`
2. Use `CircularLayout` for round display
3. Apply theme styles from `ThemeEngine`
4. Register with shell state machine
5. Handle input events and gestures

## Testing

### Unit Tests (TODO)

```bash
# Build tests
cmake -DBUILD_TESTS=ON ..
make

# Run tests
ctest --verbose
```

### Integration Tests

```bash
# Test display driver
./build/tests/test_display

# Test touch input
./build/tests/test_touch

# Test button driver
./build/tests/test_button
```

## Performance Profiling

```bash
# CPU profiling with perf
sudo apt-get install linux-perf
sudo perf record -g ./touchdown-shell
sudo perf report

# Memory profiling with valgrind
valgrind --leak-check=full ./touchdown-shell

# LVGL performance monitoring
# Enable LV_USE_PERF_MONITOR in lv_conf.h
```

## Troubleshooting

### Display not working

```bash
# Check DRM devices
ls -l /dev/dri/

# Check SPI devices
ls -l /dev/spi*

# Test DRM
sudo apt-get install drm-info
drm-info

# Check kernel logs
dmesg | grep drm
dmesg | grep spi
```

### Touch not responding

```bash
# Check I2C devices
sudo i2cdetect -y 1

# Should see device at 0x15
# Test touch events
sudo evtest
```

### Services failing to start

```bash
# Check service status
systemctl status touchdown-shell.service

# View full logs
journalctl -xe -u touchdown-shell.service

# Check D-Bus
dbus-monitor --system

# Verify permissions
ls -l /usr/bin/touchdown-*
```

## Contributing

1. Fork repository
2. Create feature branch
3. Follow coding style (clang-format)
4. Add tests for new features
5. Update documentation
6. Submit pull request

## Coding Standards

- C++17 standard
- RAII for resource management
- Smart pointers over raw pointers
- Consistent naming: `snake_case` for variables, `PascalCase` for classes
- Comprehensive logging for debugging
- Document public APIs with Doxygen comments
