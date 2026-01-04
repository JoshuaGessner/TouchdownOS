# TouchdownOS App Framework

## Overview

The TouchdownOS App Framework provides a comprehensive API for developing applications for TouchdownOS. Apps can be written in both C++ and Python, with full access to LVGL UI widgets, input handling, and system services.

## Architecture

### Core Components

- **TouchdownApp**: Base class for all apps with lifecycle management
- **AppManager**: Service for launching, pausing, and terminating apps
- **AppRegistry**: Factory pattern for app instantiation
- **ManifestParser**: JSON manifest validation and loading
- **Python Bindings**: pybind11-based Python API

### App Lifecycle

1. **Init**: Create UI and allocate resources
2. **Show**: Make app visible
3. **Update**: Regular updates while visible (30 FPS)
4. **Hide**: Hide app (background)
5. **Pause**: Save state, stop animations
6. **Resume**: Restore from pause
7. **Cleanup**: Free resources

## Creating a C++ App

### 1. Define Your App Class

```cpp
#include "touchdown/app/app.hpp"

class MyApp : public touchdown::app::TouchdownApp {
public:
    MyApp(const AppMetadata& metadata);
    ~MyApp() override;
    
    bool init(lv_obj_t* parent) override;
    void show() override;
    void hide() override;
    void cleanup() override;
    
    // Optional overrides
    void update(uint32_t delta_ms) override;
    bool on_touch(const TouchPoint& point) override;
    bool on_button(const ButtonEvent& event) override;
    bool on_back() override;
    
private:
    lv_obj_t* my_label_;
    lv_obj_t* my_button_;
};
```

### 2. Implement Lifecycle Methods

```cpp
MyApp::MyApp(const AppMetadata& metadata)
    : TouchdownApp(metadata)
    , my_label_(nullptr)
    , my_button_(nullptr) {
}

bool MyApp::init(lv_obj_t* parent) {
    // Create container
    create_container(parent);
    
    // Create UI
    my_label_ = lv_label_create(container_);
    lv_label_set_text(my_label_, "Hello TouchdownOS!");
    lv_obj_center(my_label_);
    
    my_button_ = lv_btn_create(container_);
    lv_obj_set_size(my_button_, 120, 50);
    lv_obj_align(my_button_, LV_ALIGN_BOTTOM_MID, 0, -20);
    
    return true;
}

void MyApp::show() {
    visible_ = true;
    lv_obj_clear_flag(container_, LV_OBJ_FLAG_HIDDEN);
}

void MyApp::hide() {
    visible_ = false;
    lv_obj_add_flag(container_, LV_OBJ_FLAG_HIDDEN);
}

void MyApp::cleanup() {
    // Cleanup resources
}
```

### 3. Register Your App

```cpp
// At end of cpp file
REGISTER_APP(MyApp, "com.example.myapp")
```

### 4. Create Manifest

Create `manifest.json`:

```json
{
  "id": "com.example.myapp",
  "name": "My App",
  "version": "1.0.0",
  "description": "Example app",
  "icon": "🚀",
  "color": "#2196F3",
  "type": "cpp",
  "main": "libtouchdown_myapp.so",
  "permissions": [],
  "author": "Your Name",
  "category": "utility"
}
```

## Creating a Python App

### 1. Import the API

```python
#!/usr/bin/env python3
import touchdown as td

class MyPythonApp(td.TouchdownApp):
    def __init__(self, metadata):
        super().__init__(metadata)
        self.counter = 0
        self.label = None
```

### 2. Implement Lifecycle

```python
def init(self, parent):
    """Initialize UI"""
    container = self.get_container()
    
    # Create label
    self.label = td.Widget.create_label(container, "Python App")
    td.Widget.align(self.label, td.ALIGN_CENTER, 0, 0)
    td.Widget.set_style_text_color(self.label, 0xFFFFFF)
    
    # Create button
    btn = td.Widget.create_button(container, "Click Me")
    td.Widget.set_size(btn, 120, 50)
    td.Widget.align(btn, td.ALIGN_BOTTOM_MID, 0, -20)
    
    return True

def show(self):
    td.Widget.clear_flag(self.get_container(), td.OBJ_FLAG_HIDDEN)

def hide(self):
    td.Widget.add_flag(self.get_container(), td.OBJ_FLAG_HIDDEN)

def on_touch(self, point):
    if point.type == td.TouchEventType.TAP:
        self.counter += 1
        td.Widget.set_text(self.label, f"Count: {self.counter}")
        return True
    return False

def on_button(self, event):
    if event.type == td.ButtonEventType.SINGLE_PRESS:
        self.request_close()
        return True
    return False

def cleanup(self):
    pass
```

### 3. Create Manifest

```json
{
  "id": "com.example.pythonapp",
  "name": "Python App",
  "version": "1.0.0",
  "description": "Python example",
  "icon": "🐍",
  "color": "#4CAF50",
  "type": "python",
  "main": "myapp.py",
  "permissions": [],
  "author": "Your Name",
  "category": "utility"
}
```

## Python API Reference

### TouchdownApp Class

- `init(parent)`: Initialize app with LVGL parent container
- `show()`: Make app visible
- `hide()`: Hide app
- `update(delta_ms)`: Called every frame while visible
- `on_touch(point)`: Handle touch events
- `on_button(event)`: Handle button events
- `on_back()`: Handle back gesture (return False to close)
- `cleanup()`: Free resources
- `get_metadata()`: Get app metadata
- `get_container()`: Get root LVGL container (as intptr)
- `has_permission(name)`: Check if permission granted
- `request_close()`: Ask shell to close app

### Widget Class (Static Methods)

#### Widget Creation

- `create_label(parent, text)`: Create label widget
- `create_button(parent, text)`: Create button with label
- `create_arc(parent)`: Create arc (circular progress/gauge)
- `create_slider(parent)`: Create slider
- `create_list(parent)`: Create scrollable list

#### Widget Styling

- `set_text(obj, text)`: Set label text
- `set_size(obj, w, h)`: Set widget size
- `set_pos(obj, x, y)`: Set absolute position
- `align(obj, align, x_ofs, y_ofs)`: Align widget
- `set_value(obj, value)`: Set slider/arc value
- `set_style_bg_color(obj, color)`: Set background color (0xRRGGBB)
- `set_style_text_color(obj, color)`: Set text color (0xRRGGBB)

#### Widget Flags

- `add_flag(obj, flag)`: Add flag (e.g., OBJ_FLAG_HIDDEN)
- `clear_flag(obj, flag)`: Clear flag

### Constants

#### Alignment

- `ALIGN_CENTER`
- `ALIGN_TOP_LEFT`, `ALIGN_TOP_MID`, `ALIGN_TOP_RIGHT`
- `ALIGN_BOTTOM_LEFT`, `ALIGN_BOTTOM_MID`, `ALIGN_BOTTOM_RIGHT`
- `ALIGN_LEFT_MID`, `ALIGN_RIGHT_MID`

#### Flags

- `OBJ_FLAG_HIDDEN`: Widget is hidden
- `OBJ_FLAG_CLICKABLE`: Widget can be clicked
- `OBJ_FLAG_SCROLLABLE`: Widget can scroll

#### Event Types

**TouchEventType**:
- `PRESS`, `RELEASE`, `MOVE`
- `TAP`, `LONG_PRESS`
- `SWIPE_LEFT`, `SWIPE_RIGHT`, `SWIPE_UP`, `SWIPE_DOWN`

**ButtonEventType**:
- `SINGLE_PRESS`, `DOUBLE_PRESS`, `LONG_PRESS`, `RELEASE`

## Manifest Schema

```json
{
  "id": "string (required, unique identifier)",
  "name": "string (required, display name)",
  "version": "string (required, semver X.Y.Z)",
  "description": "string (optional)",
  "icon": "string (emoji or path)",
  "color": "string (hex #RRGGBB)",
  "type": "cpp|python (required)",
  "main": "string (entry point: .so or .py)",
  "permissions": ["array of permission strings"],
  "author": "string",
  "category": "string"
}
```

### Available Permissions

- `system.settings`: Modify system settings
- `system.stats`: Read system statistics
- `power.control`: Control power features
- `network.access`: Network operations
- `bluetooth.access`: Bluetooth operations
- `storage.read`: Read files
- `storage.write`: Write files

## Input Handling

### Touch Events

Apps receive touch events via `on_touch(point)`:

```python
def on_touch(self, point):
    if point.type == td.TouchEventType.TAP:
        print(f"Tapped at ({point.x}, {point.y})")
        return True  # Event handled
    return False  # Event not handled
```

### Button Events

Physical button presses via `on_button(event)`:

```python
def on_button(self, event):
    if event.type == td.ButtonEventType.SINGLE_PRESS:
        self.request_close()
        return True
    return False
```

### Back Navigation

Swipe-down or button press triggers `on_back()`:

```python
def on_back(self):
    # Save state, confirm exit, etc.
    return False  # Return False to allow close
```

## Best Practices

### Performance

1. Keep `update()` lightweight (<16ms per frame)
2. Use LVGL animations for smooth transitions
3. Avoid blocking operations in main thread
4. Batch LVGL operations

### Memory Management

1. Clean up LVGL objects in `cleanup()`
2. Don't store large data in memory
3. Use lazy loading for resources

### UI Design

1. Design for 240x240 circular display
2. Use circular-aware layouts
3. Support both light and dark themes
4. Keep touch targets ≥40px
5. Provide visual feedback

### Error Handling

1. Return `false` from `init()` on failure
2. Handle missing permissions gracefully
3. Log errors for debugging

## Building Apps

### C++ Apps

```bash
# Add to apps/CMakeLists.txt
add_library(touchdown_myapp SHARED
    myapp.cpp
)
target_link_libraries(touchdown_myapp
    touchdown_app
    lvgl
)
```

### Python Apps

Just create `.py` file - no compilation needed.

## Installation

Apps are installed to `/usr/share/touchdown/apps/`:

```
/usr/share/touchdown/apps/
├── com.example.myapp/
│   ├── manifest.json
│   ├── libtouchdown_myapp.so  (C++)
│   └── icon.png (optional)
└── com.example.pythonapp/
    ├── manifest.json
    ├── myapp.py
    └── requirements.txt (optional)
```

## Example Apps

See:
- `src/apps/settings_app.cpp` - Settings app (C++)
- `src/apps/system_info_app.cpp` - System info (C++)
- `examples/python/example_app.py` - Counter app (Python)

## Troubleshooting

### App Won't Launch

1. Check manifest.json is valid
2. Verify app is registered (C++) or file exists (Python)
3. Check permissions in manifest
4. View logs: `journalctl -u touchdown-shell`

### UI Not Appearing

1. Ensure `create_container()` is called
2. Check objects are created on `container_`
3. Verify `show()` clears HIDDEN flag
4. Test with simple label first

### Python Import Error

1. Check `touchdown` module is installed
2. Verify Python 3.x is used
3. Ensure pybind11 was compiled

## API Evolution

The TouchdownOS app API is under active development. Current version: 0.1.0

Breaking changes will be documented in release notes.
