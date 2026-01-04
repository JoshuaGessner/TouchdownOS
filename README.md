# TouchdownOS

A custom wearable Linux operating environment with a fully custom graphical shell built entirely using LVGL, running on headless Linux with no desktop manager or window system.

## Hardware Target

- **Compute:** Raspberry Pi Zero 2 W
- **Display:** 1.28" round LCD, 240x240 resolution, GC9A01 driver (SPI)
- **Touch:** Capacitive touch, CST816S controller (I2C)
- **Input:** One physical button with single/double/long press detection
- **Wireless:** WiFi and Bluetooth

## Architecture

TouchdownOS is a layered system:

1. **Kernel/Drivers Layer** - DRM/KMS display, I2C touch, GPIO button
2. **System Services Layer** - Power management, input handling, network, D-Bus IPC
3. **LVGL Shell Layer** - Core UI, theme engine, home screen/watch face
4. **Applications Layer** - C++ system apps and Python user apps

## Build Requirements

### Cross-Compilation Toolchain
```bash
sudo apt-get install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf
```

### Dependencies
```bash
sudo apt-get install cmake pkg-config \
    libdrm-dev libdbus-1-dev libsystemd-dev \
    libudev-dev libinput-dev \
    python3-dev python3-pip
```

### Initialize Submodules
```bash
git submodule update --init --recursive
```

## Building

### Native Build (for development)
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Cross-Compilation for Pi Zero 2 W
```bash
mkdir build-arm && cd build-arm
cmake -DCROSS_COMPILE=ON ..
make -j$(nproc)
sudo make install DESTDIR=./rootfs
```

### Create Debian Package
```bash
cd build-arm
cpack
```

## Installation on Raspberry Pi

1. Flash Raspberry Pi OS Lite to SD card
2. Copy and install the .deb package:
   ```bash
   sudo dpkg -i touchdownos_0.1.0_armhf.deb
   ```
3. Enable device tree overlays in `/boot/config.txt`
4. Reboot

## Development

See [docs/development.md](docs/development.md) for detailed development guidelines.

## License

MIT License - See LICENSE file for details
