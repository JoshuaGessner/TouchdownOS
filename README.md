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
2. **System Services Layer** - Power management, input handling, app manager, D-Bus IPC
3. **LVGL Shell Layer** - Core UI, theme engine, home screen, app launcher
4. **Applications Layer** - C++ system apps and Python user apps with full API access

### App Framework

TouchdownOS provides a comprehensive app framework with:

- **TouchdownApp Base Class**: Full lifecycle management (init/show/hide/pause/resume/update)
- **Python Bindings**: pybind11-based Python API for rapid development
- **LVGL Widget API**: Complete access to LVGL UI widgets and styling
- **Input Handling**: Touch gestures, button events, back navigation
- **Manifest System**: JSON-based app metadata with permissions
- **AppManager Service**: Process management and resource isolation

See [docs/app-framework.md](docs/app-framework.md) for complete API documentation.

## Build Requirements

1) Enable armhf architecture (needed for ARM libs):
```bash
sudo dpkg --add-architecture armhf
sudo apt-get update
```

2) Install host toolchains and common deps:
```bash
sudo apt-get install \
    build-essential cmake pkg-config git \
    gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf \
    libdrm-dev libdbus-1-dev libsystemd-dev libudev-dev libinput-dev \
    python3-dev python3-pip pybind11-dev
```

3) Install ARM target libs (for pkg-config when cross-compiling):
```bash
sudo apt-get install libdrm-dev:armhf libdbus-1-dev:armhf libsystemd-dev:armhf
```

4) Initialize submodules (LVGL, drivers, msgpack):
```bash
git submodule update --init --recursive
```

## Building

### Cross-Compilation for Pi Zero 2 W (recommended for target testing)

Fresh configure/build (clears any stale CMake cache):
```bash
cd /home/josh/dev/TouchdownOS
rm -rf build-arm
mkdir build-arm && cd build-arm
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-pi-toolchain.cmake ..
make -j$(nproc)
```

Create Debian package:
```bash
cd /home/josh/dev/TouchdownOS/build-arm
cpack
# Produces: touchdownos_0.1.0_armhf.deb
```

Install to staging directory (optional for inspecting layout):
```bash
cd /home/josh/dev/TouchdownOS/build-arm
sudo make install DESTDIR=./rootfs
```

### Native Build (x86_64 dev/test)
```bash
cd /home/josh/dev/TouchdownOS
rm -rf build
mkdir build && cd build
cmake ..
make -j$(nproc)
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

### Creating Apps

TouchdownOS supports both C++ and Python apps with the same powerful API:

**C++ Example:**
```cpp
class MyApp : public touchdown::app::TouchdownApp {
public:
    bool init(lv_obj_t* parent) override {
        create_container(parent);
        lv_obj_t* label = lv_label_create(container_);
        lv_label_set_text(label, "Hello!");
        return true;
    }
    // ... lifecycle methods
};
REGISTER_APP(MyApp, "com.example.myapp")
```

**Python Example:**
```python
import touchdown as td

class MyApp(td.TouchdownApp):
    def init(self, parent):
        container = self.get_container()
        label = td.Widget.create_label(container, "Hello!")
        td.Widget.align(label, td.ALIGN_CENTER, 0, 0)
        return True
```

See [docs/app-framework.md](docs/app-framework.md) for complete development guide.

### Documentation

- [Architecture](docs/architecture.md) - System design and components
- [Building](docs/building.md) - Detailed build instructions and troubleshooting
- [Development](docs/development.md) - Development workflow and guidelines
- [App Framework](docs/app-framework.md) - Complete app API reference

## License

MIT License - See LICENSE file for details
