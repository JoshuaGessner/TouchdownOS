# TouchdownOS Applications

This directory contains system applications and examples for TouchdownOS.

## Directory Structure

```
apps/
├── manifests/          # App manifest files
│   ├── settings.json   # Settings app manifest
│   ├── info.json       # System info app manifest
│   └── example.json    # Example Python app manifest
└── CMakeLists.txt
```

## System Apps

### Settings (`settings`)

- **Description**: System settings and configuration
- **Type**: C++ (built-in)
- **Features**:
  - Theme toggle (dark/light)
  - Brightness adjustment
  - About screen

### System Info (`info`)

- **Description**: Display system statistics
- **Type**: C++ (built-in)
- **Features**:
  - CPU usage
  - Memory usage
  - System uptime
  - Temperature monitoring
  - Auto-updates every 2 seconds

### Example Python App (`com.touchdown.example`)

- **Description**: Demonstrates Python API
- **Type**: Python
- **Location**: `examples/python/example_app.py`
- **Features**:
  - Simple counter
  - Button interaction
  - Touch handling

## Creating New Apps

See [docs/app-framework.md](../docs/app-framework.md) for complete guide.

### Quick Start (C++)

1. Create app class inheriting from `touchdown::app::TouchdownApp`
2. Implement lifecycle methods
3. Register with `REGISTER_APP(MyApp, "app_id")`
4. Create `manifest.json`
5. Add to CMakeLists.txt

### Quick Start (Python)

1. Create class inheriting from `touchdown.TouchdownApp`
2. Implement lifecycle methods
3. Create `manifest.json`
4. Place in `/usr/share/touchdown/apps/`

## Manifest Files

Manifests are JSON files describing app metadata:

```json
{
  "id": "unique.app.id",
  "name": "Display Name",
  "version": "1.0.0",
  "description": "App description",
  "icon": "🚀",
  "color": "#2196F3",
  "type": "cpp|python",
  "main": "entry_point",
  "permissions": [],
  "author": "Author Name",
  "category": "category"
}
```

## Installation

Apps are installed to:
- C++ apps: `/usr/lib/touchdown/` (shared libraries)
- Python apps: `/usr/share/touchdown/apps/`
- Manifests: `/usr/share/touchdown/apps/{app_id}/manifest.json`

## App Categories

- `system`: System utilities
- `tools`: User tools
- `games`: Games
- `examples`: Example/demo apps
- `other`: Uncategorized

## Permissions

Available permissions:
- `system.settings`: Modify system settings
- `system.stats`: Read system statistics
- `power.control`: Power management
- `network.access`: Network operations
- `bluetooth.access`: Bluetooth operations
- `storage.read`: Read files
- `storage.write`: Write files

## Testing Apps

### C++ Apps

```bash
# Build
mkdir build && cd build
cmake -DCROSS_COMPILE=ON ..
make

# Install
sudo make install

# Run shell
touchdown-shell
```

### Python Apps

```bash
# Install module
sudo pip3 install /path/to/touchdown.so

# Copy app
sudo cp -r myapp/ /usr/share/touchdown/apps/

# Run shell
touchdown-shell
```

## Debugging

View shell logs:
```bash
journalctl -u touchdown-shell -f
```

Enable debug logging in `/etc/touchdown/shell.conf`:
```ini
[logging]
level = debug
```
