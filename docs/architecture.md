# TouchdownOS Architecture

## Overview

TouchdownOS is a custom wearable Linux operating environment built on a headless Linux base with LVGL as the sole UI layer. It's designed for the Raspberry Pi Zero 2 W with a 1.28" round 240x240 display.

## System Layers

```
┌─────────────────────────────────────────┐
│         Python User Applications        │
│      (Isolated processes via IPC)       │
├─────────────────────────────────────────┤
│         Application Framework           │
│   (App lifecycle, manifest, manager)    │
├─────────────────────────────────────────┤
│           LVGL Shell Core               │
│   (Home screen, launcher, theme)        │
├─────────────────────────────────────────┤
│          System Services                │
│  (Power, Input, Battery, Network)       │
│         (D-Bus communication)           │
├─────────────────────────────────────────┤
│      Hardware Abstraction Layer         │
│   (Display, Touch, Button drivers)      │
├─────────────────────────────────────────┤
│            LVGL on DRM/KMS              │
│      (Direct rendering, no X11)         │
├─────────────────────────────────────────┤
│          Linux Kernel + Drivers         │
│   (drm_mipi_dbi, CST816S, GPIO)        │
└─────────────────────────────────────────┘
```

## Core Components

### 1. Hardware Drivers (`src/drivers/`)

**DisplayDriver** (`display_driver.cpp`)
- DRM/KMS display interface
- GC9A01 240x240 round LCD via drm_mipi_dbi
- Circular viewport masking
- Power management (DPMS)
- Double buffering for smooth rendering

**TouchDriver** (`touch_driver.cpp`)
- CST816S I2C capacitive touch controller
- Coordinate transformation for circular display
- Gesture detection (tap, long press, swipes)
- Touch event abstraction for LVGL

**ButtonDriver** (`button_driver.cpp`)
- GPIO button input (evdev)
- Debouncing and gesture detection
- Single/double/long press recognition
- Configurable timing thresholds

### 2. System Services (`src/services/`)

All services use D-Bus for IPC and systemd for lifecycle management.

**PowerService** (`power_service.cpp`)
- Display power control (screen on/off via DPMS)
- CPU frequency scaling (schedutil/powersave governors)
- Idle timeout and screen blanking
- Battery monitoring (future)
- Systemd integration with watchdog

**InputService** (`input_service.cpp`)
- Aggregates touch and button input
- Broadcasts input events via D-Bus signals
- Provides input state queries
- Coordinates with power service for wake-on-touch

**D-Bus Interfaces**
- `org.touchdown.Power` - Power management
- `org.touchdown.Input` - Input aggregation
- `org.touchdown.Shell` - Shell coordination
- `org.touchdown.AppManager` - Application lifecycle (future)

### 3. LVGL Shell (`src/shell/`)

**Shell** (`shell.cpp`)
- Main coordinator for UI and input
- State machine (HOME, APP_LAUNCHER, APP_RUNNING)
- Integrates all hardware drivers
- LVGL event loop handler
- Time updates and system status

**ThemeEngine** (`theme_engine.cpp`)
- Global color palette management
- Dark/light theme support
- Smooth theme transitions
- Consistent styling for all UI elements
- LVGL theme integration

**HomeScreen** (`home_screen.cpp`)
- Primary watch face display
- Time and date widgets
- Status indicators (battery, WiFi, Bluetooth)
- Quick actions (future)
- Swipe gestures to launcher

**AppLauncher** (`app_launcher.cpp`)
- Circular app grid layout
- Touch-optimized app buttons
- Smooth show/hide animations
- App launch coordination
- Dynamic app list management

**CircularLayout** (`circular_layout.cpp`)
- Round display geometry utilities
- Safe zone calculations
- Arc-based positioning
- Circular masking helpers

### 4. Application Framework (Future - `apps/`)

**App Base Classes**
- `TouchdownApp` (C++ base class)
- Python bindings via pybind11
- Lifecycle hooks (init, pause, resume, destroy)
- Standard UI patterns and themes

**App Manager Service**
- Process management for Python apps
- MessagePack-based IPC protocol
- Resource limits and sandboxing
- App manifest parsing (JSON/TOML)
- Installation and removal

**App Manifest Format**
```json
{
  "id": "com.example.myapp",
  "name": "My App",
  "version": "1.0.0",
  "icon": "myapp_icon.png",
  "type": "python",
  "main": "main.py",
  "permissions": ["network", "storage"]
}
```

## Communication Patterns

### Driver → Service → Shell

1. **Input Flow**
   ```
   CST816S → TouchDriver → InputService (D-Bus) → Shell → LVGL
   GPIO → ButtonDriver → InputService (D-Bus) → Shell
   ```

2. **Power Management**
   ```
   Shell (idle) → PowerService → DisplayDriver (DPMS)
   Shell (activity) → PowerService (reset timer)
   ```

### Shell ↔ Apps (Future)

**IPC via D-Bus + MessagePack**
- App spawned as separate process
- D-Bus session bus for control messages
- MessagePack for structured data (UI updates)
- Unix domain sockets for low-latency streaming

**App Lifecycle**
```
AppManager → spawn Python process
  → App connects to D-Bus
  → Shell sends init message
  → App renders UI (via IPC)
  → User input forwarded to app
  → App sends state updates
  → Shell pauses/resumes/terminates
```

## Display Rendering Pipeline

```
LVGL UI Components
       ↓
  Theme Styles Applied
       ↓
  Circular Layout Constraints
       ↓
  LVGL Draw Buffer (RAM)
       ↓
  Display Driver flush_cb()
       ↓
  DRM dumb buffer (mmap)
       ↓
  DRM page flip (vsync)
       ↓
  GC9A01 SPI Transfer
       ↓
  Physical Display
```

**Performance Targets**
- 30 FPS for animations
- < 10ms input latency
- < 4ms frame flush time (240x240x2 bytes = 115KB at 32MHz SPI)

## Power States

| State       | Display | CPU Governor | Touch | Button |
|-------------|---------|--------------|-------|--------|
| ACTIVE      | On      | schedutil    | ✓     | ✓      |
| SCREEN_OFF  | Off     | powersave    | ✓     | ✓      |
| SUSPENDED   | Off     | powersave    | ✗     | ✓      |
| SHUTDOWN    | -       | -            | -     | -      |

## File System Layout

```
/usr/bin/
  touchdown-shell              # Main UI shell
  touchdown-power-service      # Power management
  touchdown-input-service      # Input handling

/etc/touchdown/
  shell.conf                   # Shell configuration
  apps/                        # App configurations

/usr/share/touchdown/
  apps/                        # Installed applications
    settings/
    system-info/
  themes/                      # Theme resources
  assets/                      # Icons, fonts

/var/lib/touchdown/
  apps/                        # App data storage
  state/                       # System state

/etc/systemd/system/
  touchdown-shell.service
  touchdown-power.service
  touchdown-input.service
  touchdown-app@.service
```

## Build System

**Cross-Compilation Toolchain**
- Target: `arm-linux-gnueabihf`
- Architecture: ARMv7-A (Cortex-A53)
- CMake-based build system
- Automatic dependency detection

**Dependencies**
- LVGL v9.0 (submodule)
- libdrm (DRM/KMS)
- libdbus-1 (D-Bus IPC)
- libsystemd (systemd integration)
- msgpack-c (MessagePack serialization)

**Build Artifacts**
- Static libraries: `libTouchdown-core.a`, `libTouchdown-drivers.a`, `libTouchdown-services.a`
- Executables: `touchdown-shell`, service binaries
- Debian package: `touchdownos_0.1.0_armhf.deb`

## Security Considerations

**Service Isolation**
- Services run with minimal privileges
- `ProtectSystem=strict`, `ProtectHome=yes`
- Capability-based access control
- Resource limits (CPU, memory, tasks)

**App Sandboxing** (Future)
- Separate processes per app
- D-Bus policy enforcement
- Filesystem access restrictions
- Network access control

## Future Enhancements

1. **Hardware Support**
   - Wardriving modules
   - Radio peripherals
   - Additional sensors

2. **System Features**
   - A/B partition OTA updates
   - Read-only rootfs with overlayfs
   - Plymouth boot splash
   - Web-based configuration UI

3. **Application Ecosystem**
   - App store/repository
   - Developer SDK
   - Visual app builder
   - More built-in apps

## Performance Optimization

**Display**
- Partial updates via damage tracking
- GPU acceleration where available
- Efficient circular clipping
- Minimal overdraw

**Power**
- Aggressive CPU scaling when idle
- Display blanking after timeout
- Selective peripheral power-down
- Wake-on-interrupt

**Memory**
- Static allocation where possible
- LVGL memory pool tuning
- Efficient app lifecycle management
- Resource cleanup on app exit
