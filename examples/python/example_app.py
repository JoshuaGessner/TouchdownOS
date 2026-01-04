#!/usr/bin/env python3
"""
Example Python app for TouchdownOS
Demonstrates the Python API for creating apps
"""

import touchdown as td

class ExamplePythonApp(td.TouchdownApp):
    def __init__(self, metadata):
        super().__init__(metadata)
        self.counter = 0
        self.label = None
        self.button = None
        
    def init(self, parent):
        """Initialize the app UI"""
        print(f"Initializing {self.get_metadata().name}")
        
        # Create container
        container = self.get_container()
        
        # Title
        title = td.Widget.create_label(container, "Python App")
        td.Widget.align(title, td.ALIGN_TOP_MID, 0, 20)
        td.Widget.set_style_text_color(title, 0xFFFFFF)
        
        # Counter label
        self.label = td.Widget.create_label(container, f"Count: {self.counter}")
        td.Widget.align(self.label, td.ALIGN_CENTER, 0, -20)
        td.Widget.set_style_text_color(self.label, 0x00FF00)
        
        # Button
        self.button = td.Widget.create_button(container, "Increment")
        td.Widget.set_size(self.button, 120, 50)
        td.Widget.align(self.button, td.ALIGN_CENTER, 0, 40)
        td.Widget.set_style_bg_color(self.button, 0x2196F3)
        
        return True
        
    def show(self):
        """Show the app"""
        td.Widget.clear_flag(self.get_container(), td.OBJ_FLAG_HIDDEN)
        
    def hide(self):
        """Hide the app"""
        td.Widget.add_flag(self.get_container(), td.OBJ_FLAG_HIDDEN)
        
    def update(self, delta_ms):
        """Update app state (called regularly)"""
        # Could update animations, refresh data, etc.
        pass
        
    def on_touch(self, point):
        """Handle touch events"""
        if point.type == td.TouchEventType.TAP:
            # Check if button was tapped (simplified)
            self.counter += 1
            td.Widget.set_text(self.label, f"Count: {self.counter}")
            return True
        return False
        
    def on_button(self, event):
        """Handle physical button events"""
        if event.type == td.ButtonEventType.SINGLE_PRESS:
            # Back to home
            self.request_close()
            return True
        return False
        
    def cleanup(self):
        """Cleanup resources"""
        print(f"Cleaning up {self.get_metadata().name}")


def main():
    """Entry point for Python app"""
    metadata = td.AppMetadata()
    metadata.id = "com.touchdown.example"
    metadata.name = "Example App"
    metadata.version = "1.0.0"
    metadata.description = "Example Python app"
    metadata.icon = "⚡"
    
    app = ExamplePythonApp(metadata)
    # App lifecycle is managed by the shell
    # This would be called from C++ side


if __name__ == "__main__":
    main()
